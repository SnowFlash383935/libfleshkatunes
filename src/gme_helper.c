#define _ISOC99_SOURCE
#include "gme_helper.h"
#include "chiptune.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Вспомогательные функции для записи заголовка WAV в формате Little-Endian */
static void write_le32(FILE *f, uint32_t val) {
    uint8_t buf[4] = {
        (uint8_t)(val & 0xFF),
        (uint8_t)((val >> 8) & 0xFF),
        (uint8_t)((val >> 16) & 0xFF),
        (uint8_t)((val >> 24) & 0xFF)
    };
    fwrite(buf, 1, 4, f);
}

static void write_le16(FILE *f, uint16_t val) {
    uint8_t buf[2] = {
        (uint8_t)(val & 0xFF),
        (uint8_t)((val >> 8) & 0xFF)
    };
    fwrite(buf, 1, 2, f);
}

/* * Универсальная функция рендеринга любого формата (включая SPC через GME-фасад).
 * Больше не содержит специфичных для GME заголовочных вызовов в цикле записи.
 */
int render_gme_to_wav(const char *input_path, const char *output_wav_path, int duration_sec) {
    const long sample_rate = 44100;
    const int track_index = 0;
    const int chan_count = 2;
    const size_t buf_size = 16384; /* Количество сэмплов за один проход рендеринга */

    /* Открываем файл через единый фасад библиотеки */
    ChiptunePlayer* player = chiptune_open_file(input_path, sample_rate);
    if (!player) {
        fprintf(stderr, "Chiptune Error: Failed to open or recognize file: %s\n", input_path);
        return -1;
    }

    /* Настраиваем параметры воспроизведения через абстрактный интерфейс */
    if (chiptune_start_track(player, track_index) != 0) {
        fprintf(stderr, "Chiptune Error: Failed to start track %d\n", track_index);
        chiptune_close(player);
        return -1;
    }

    chiptune_set_looping(player, 0); /* Отключаем автозацикливание для детерминированного рендеринга */

    /* Открываем целевой WAV-файл для записи PCM-потока */
    FILE *file = fopen(output_wav_path, "wb");
    if (!file) {
        fprintf(stderr, "Error: Couldn't open WAVE file for writing: %s\n", output_wav_path);
        chiptune_close(player);
        return -1;
    }

    /* Резервируем пространство под 44-байтный заголовок RIFF WAVE */
    uint8_t placeholder[44] = {0};
    fwrite(placeholder, 1, 44, file);

    long total_samples = 0;
    long max_time_ms = (long)duration_sec * 1000L;
    short* buf = (short*)malloc(buf_size * sizeof(short));
    
    if (!buf) {
        fprintf(stderr, "Error: Memory allocation failed for audio buffer\n");
        fclose(file);
        chiptune_close(player);
        return -1;
    }

    /* Основной конвейер рендеринга с использованием полиморфного метода */
    while (chiptune_tell(player) < max_time_ms) {
        int rendered = chiptune_play(player, buf, (int)buf_size);
        if (rendered < 0) {
            fprintf(stderr, "Chiptune Error: Render failure during playback loop\n");
            break;
        }

        /* Запись сэмплов в порядке байтов Little-Endian */
        for (int i = 0; i < rendered; i++) {
            write_le16(file, (uint16_t)buf[i]);
        }
        total_samples += rendered;
    }

    free(buf);

    /* Формирование и дописывание актуального заголовка WAV в начало файла */
    fseek(file, 0, SEEK_SET);

    uint32_t data_size = total_samples * sizeof(short);
    uint32_t file_size = 44 - 8 + data_size;
    uint32_t byte_rate = sample_rate * chan_count * sizeof(short);
    uint16_t block_align = chan_count * sizeof(short);

    fwrite("RIFF", 1, 4, file);
    write_le32(file, file_size);
    fwrite("WAVE", 1, 4, file);
    fwrite("fmt ", 1, 4, file);
    write_le32(file, 16);          /* Размер структуры PCM */
    write_le16(file, 1);           /* Аудиоформат (1 = PCM) */
    write_le16(file, chan_count);
    write_le32(file, sample_rate);
    write_le32(file, byte_rate);
    write_le16(file, block_align);
    write_le16(file, 16);          /* Битность (16 бит) */
    fwrite("data", 1, 4, file);
    write_le32(file, data_size);

    fclose(file);
    chiptune_close(player); /* Безопасное полиморфное освобождение ресурсов бэкенда */
    return 0;
}
