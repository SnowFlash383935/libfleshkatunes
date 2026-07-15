#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h>

/* Вспомогательная функция для чтения 32-битного числа в little-endian */
static uint32_t read_le32(const uint8_t *buf) {
    return ((uint32_t)buf[0]) |
           ((uint32_t)buf[1] << 8) |
           ((uint32_t)buf[2] << 16) |
           ((uint32_t)buf[3] << 24);
}

/* Вспомогательная функция для извлечения значения тега по ключу из метаданных */
static char* extract_tag_value(const char *metadata, const char *key) {
    char search_buf[64];
    snprintf(search_buf, sizeof(search_buf), "\n%s=", key);
    
    const char *ptr = strstr(metadata, search_buf);
    if (!ptr) {
        // Проверяем, если ключ идет в самом начале текста (без переноса строки перед ним)
        snprintf(search_buf, sizeof(search_buf), "%s=", key);
        if (strncmp(metadata, search_buf, strlen(search_buf)) == 0) {
            ptr = metadata;
        } else {
            return NULL;
        }
    } else {
        ptr += 1; // Пропускаем символ '\n'
    }

    const char *val_ptr = strchr(ptr, '=');
    if (!val_ptr) return NULL;
    val_ptr++; // переходим к значению

    // Находим конец строки
    const char *end_ptr = strchr(val_ptr, '\n');
    size_t len = 0;
    if (end_ptr) {
        if (end_ptr > val_ptr && *(end_ptr - 1) == '\r') {
            len = (end_ptr - 1) - val_ptr;
        } else {
            len = end_ptr - val_ptr;
        }
    } else {
        len = strlen(val_ptr);
    }

    char *val = malloc(len + 1);
    if (!val) return NULL;
    memcpy(val, val_ptr, len);
    val[len] = '\0';
    return val;
}

/**
 * Сканирует файл и возвращает массив имен библиотек (_lib1, _lib2... или _lib).
 * Количество найденных элементов записывается в out_count.
 */
char** psf_get_lib_names(const char* filename, int *out_count) {
    if (!filename || !out_count) return NULL;
    *out_count = 0;

    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    // 1. Читаем сигнатуру "PSF" (3 байта) и версию (1 байт)
    char sig[3];
    if (fread(sig, 1, 3, f) != 3 || memcmp(sig, "PSF", 3) != 0) {
        fclose(f);
        return NULL;
    }

    uint8_t version;
    if (fread(&version, 1, 1, f) != 1) {
        fclose(f);
        return NULL;
    }

    // 2. Читаем размеры и CRC32 (по 4 байта)
    uint8_t buf4[4];
    if (fread(buf4, 1, 4, f) != 4) { fclose(f); return NULL; } // reserved_size

    if (fread(buf4, 1, 4, f) != 4) { fclose(f); return NULL; }
    uint32_t exe_compressed_size = read_le32(buf4);

    if (fread(buf4, 1, 4, f) != 4) { fclose(f); return NULL; }
    uint32_t expected_crc = read_le32(buf4);

    // 3. Читаем сжатый блок данных
    uint8_t *exe_compressed = malloc(exe_compressed_size);
    if (!exe_compressed) {
        fclose(f);
        return NULL;
    }

    if (fread(exe_compressed, 1, exe_compressed_size, f) != exe_compressed_size) {
        free(exe_compressed);
        fclose(f);
        return NULL;
    }

    // 4. Проверяем CRC32 через zlib
    uLong computed_crc = crc32(0L, Z_NULL, 0);
    computed_crc = crc32(computed_crc, exe_compressed, exe_compressed_size);
    free(exe_compressed);

    if ((uint32_t)computed_crc != expected_crc) {
        fclose(f);
        return NULL;
    }

    // 5. Читаем метаданные до конца файла
    long current_pos = ftell(f);
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long end_pos = ftell(f);
    long meta_size = end_pos - current_pos;
    
    if (fseek(f, current_pos, SEEK_SET) != 0 || meta_size <= 0) {
        fclose(f);
        return NULL;
    }

    char *metadata = malloc(meta_size + 1);
    if (!metadata) {
        fclose(f);
        return NULL;
    }

    if (fread(metadata, 1, meta_size, f) != (size_t)meta_size) {
        free(metadata);
        fclose(f);
        return NULL;
    }
    metadata[meta_size] = '\0';
    fclose(f);

    // 6. Собираем зависимости: сначала ищем _lib1, _lib2, _lib3...
    int max_libs = 4;
    char **lib_names = malloc(max_libs * sizeof(char*));
    if (!lib_names) {
        free(metadata);
        return NULL;
    }
    int count = 0;

    int n = 1;
    while (1) {
        char key_buf[32];
        snprintf(key_buf, sizeof(key_buf), "_lib%d", n);
        char *val = extract_tag_value(metadata, key_buf);
        if (!val) break;

        if (count >= max_libs) {
            max_libs *= 2;
            char **new_ptr = realloc(lib_names, max_libs * sizeof(char*));
            if (!new_ptr) {
                for (int i = 0; i < count; i++) free(lib_names[i]);
                free(lib_names);
                free(metadata);
                return NULL;
            }
            lib_names = new_ptr;
        }
        lib_names[count++] = val;
        n++;
    }

    // Если нумерованных тегов нет, проверяем классический одиночный тег _lib
    if (count == 0) {
        char *val = extract_tag_value(metadata, "_lib");
        if (val) {
            lib_names[count++] = val;
        }
    }

    free(metadata);

    if (count == 0) {
        free(lib_names);
        return NULL;
    }

    *out_count = count;
    return lib_names;
}

/* Функция для освобождения памяти массива имен либ */
void psf_free_lib_names(char **lib_names, int count) {
    if (!lib_names) return;
    for (int i = 0; i < count; i++) {
        free(lib_names[i]);
    }
    free(lib_names);
}
