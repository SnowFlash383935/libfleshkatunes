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

const char* psf_get_lib_name(const char* filename) {
    if (!filename) return NULL;

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
    if (fread(buf4, 1, 4, f) != 4) { fclose(f); return NULL; }
    // reserved_size игнорируем или сохраняем: uint32_t reserved_size = read_le32(buf4);

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
        // Ошибка CRC — возвращаем NULL, как и заказывали
        fclose(f);
        return NULL;
    }

    // 5. Читаем оставшуюся часть файла (метаданные)
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

    // 6. Ищем тег "_lib=" внутри текста безопасным образом
    const char *lib_key = "_lib=";
    char *lib_ptr = strstr(metadata, lib_key);
    char *lib_val = NULL;

    if (lib_ptr) {
        lib_ptr += strlen(lib_key);
        // Находим конец строки с именем библиотеки
        char *end_ptr = strchr(lib_ptr, '\n');
        if (end_ptr) {
            // Убираем возможный \r (CRLF)
            if (end_ptr > lib_ptr && *(end_ptr - 1) == '\r') {
                end_ptr--;
            }
            size_t len = end_ptr - lib_ptr;
            lib_val = malloc(len + 1);
            if (lib_val) {
                memcpy(lib_val, lib_ptr, len);
                lib_val[len] = '\0';
            }
        } else {
            // Если перевода строки нет до конца файла
            lib_val = strdup(lib_ptr);
        }
    }

    free(metadata);
    return lib_val; // Будет NULL, если тег не найден, либо строка с именем либы
}
