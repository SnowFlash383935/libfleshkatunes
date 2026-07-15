#ifndef UTILITIES_H
#define UTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Парсит mini-PSF/GSF/USF/2SF файл, проверяет CRC32 сжатого блока через zlib.
 * Если CRC не совпал или файл поврежден — возвращает NULL.
 * Ищет зависимости по цепочке (_lib1, _lib2, _lib3... а также поддерживает классический _lib).
 * Количество найденных элементов записывается в out_count.
 * Возвращает динамически выделенный массив строк (char**).
 * Внимание: вызывающий код обязан освободить память с помощью psf_free_lib_names()!
 */
char** psf_get_lib_names(const char* filename, int *out_count);

/* Освобождает память, выделенную под массив имен библиотек.
 */
void psf_free_lib_names(char **lib_names, int count);

#ifdef __cplusplus
}
#endif

#endif /* UTILITIES_H */
