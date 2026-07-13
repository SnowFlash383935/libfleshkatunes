#define _ISOC99_SOURCE
#include "gme_helper.h"
#include <gme/gme.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void handle_error_internal(const char* str) {
    if (str) {
        fprintf(stderr, "GME Error: %s\n", str);
        exit(EXIT_FAILURE);
    }
}

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

int render_gme_to_wav(const char *input_path, const char *output_wav_path, int duration_sec) {
    long sample_rate = 44100;
    int track = 0;
    int chan_count = 2;

    Music_Emu* emu = NULL;
    handle_error_internal(gme_open_file(input_path, &emu, sample_rate));
    gme_set_autoload_playback_limit(emu, 0);
    gme_ignore_silence(emu, 1);

    handle_error_internal(gme_start_track(emu, track));

    FILE *file = fopen(output_wav_path, "wb");
    if (!file) {
        fprintf(stderr, "Error: Couldn't open WAVE file for writing: %s\n", output_wav_path);
        gme_delete(emu);
        return -1;
    }

    uint8_t placeholder[44] = {0};
    fwrite(placeholder, 1, 44, file);

    long total_samples = 0;
    long max_time_ms = (long)duration_sec * 1000L;

    while (gme_tell(emu) < max_time_ms) {
        #define buf_size 16384
        short buf[buf_size];

        handle_error_internal(gme_play(emu, buf_size, buf));

        for (int i = 0; i < buf_size; i++) {
            write_le16(file, (uint16_t)buf[i]);
        }
        total_samples += buf_size;
    }

    fseek(file, 0, SEEK_SET);

    uint32_t data_size = total_samples * sizeof(short);
    uint32_t file_size = 44 - 8 + data_size;
    uint32_t byte_rate = sample_rate * chan_count * sizeof(short);
    uint16_t block_align = chan_count * sizeof(short);

    fwrite("RIFF", 1, 4, file);
    write_le32(file, file_size);
    fwrite("WAVE", 1, 4, file);
    fwrite("fmt ", 1, 4, file);
    write_le32(file, 16);
    write_le16(file, 1);
    write_le16(file, chan_count);
    write_le32(file, sample_rate);
    write_le32(file, byte_rate);
    write_le16(file, block_align);
    write_le16(file, 16);
    fwrite("data", 1, 4, file);
    write_le32(file, data_size);

    fclose(file);
    gme_delete(emu);
    return 0;
}
