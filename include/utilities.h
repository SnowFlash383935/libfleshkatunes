#ifndef UTILITIES_H
#define UTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Парсит mini-PSF/GSF файл, проверяет CRC32 сжатого блока через zlib.
 * Если CRC не совпал или файл поврежден — возвращает NULL.
 * Возвращает динамически выделенную строку с именем библиотеки (_lib).
 * Внимание: вызывающий код обязан освободить память с помощью free()!
 */
const char* psf_get_lib_name(const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* UTILITIES_H */
