#include "chiptune_internal.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

/* Объявление фабричной функции GME-бэкенда для работы с памятью */
ChiptunePlayer* chiptune_gme_open_data(const void* data, size_t size, long sample_rate);

/* Кроссплатформенный аналог регистронезависимого сравнения (если под Windows, используется _stricmp) */
#if defined(_MSC_VER)
#define strcasecmp _stricmp
#endif

ChiptunePlayer* chiptune_open_file(const char* filepath, long sample_rate) {
    if (!filepath) return NULL;

    /* Читаем файл целиком в оперативную память */
    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    void *buffer = malloc(size);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    if (fread(buffer, 1, size, f) != (size_t)size) {
        free(buffer);
        fclose(f);
        return NULL;
    }
    fclose(f);

    /* Передаем полученный буфер в бэкенд открытия из памяти */
    ChiptunePlayer *player = chiptune_gme_open_data(buffer, size, sample_rate);

    /* Буфер больше не нужен, бэкенд gme_open_data копирует всё необходимое во внутренние структуры */
    free(buffer);

    return player;
}

int chiptune_start_track(ChiptunePlayer* player, int track_index) {
    if (player && player->start_track) {
        return player->start_track(player, track_index);
    }
    return -1;
}

void chiptune_set_looping(ChiptunePlayer* player, int enable) {
    if (player && player->set_looping) {
        player->set_looping(player, enable);
    }
}

int chiptune_seek(ChiptunePlayer* player, int position_ms) {
    if (player && player->seek) {
        return player->seek(player, position_ms);
    }
    return -1;
}

int chiptune_tell(const ChiptunePlayer* player) {
    if (player && player->tell) {
        return player->tell(player);
    }
    return -1;
}

int chiptune_get_info(const ChiptunePlayer* player, ChiptuneInfo* info) {
    if (player && player->get_info) {
        return player->get_info(player, info);
    }
    return -1;
}

int chiptune_play(ChiptunePlayer* player, short* buffer, int max_samples) {
    if (player && player->play) {
        return player->play(player, buffer, max_samples);
    }
    return -1;
}

void chiptune_close(ChiptunePlayer* player) {
    if (player && player->close) {
        player->close(player);
    }
}
