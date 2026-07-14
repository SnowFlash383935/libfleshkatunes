#include "chiptune_internal.h"
#include <string.h>
#include <stdlib.h>

/* Объявление фабричной функции GME-бэкенда */
ChiptunePlayer* chiptune_gme_open_file(const char* filepath, long sample_rate);

ChiptunePlayer* chiptune_open_file(const char* filepath, long sample_rate) {
    if (!filepath) return NULL;
    char *dot = strrchr(filepath, '.');
    char *extension = dot + 1;
    /* Диспетчер форматов: для любых файлов пока используем GME */
    /* В будущем здесь будет проверка расширения (.sid -> sid_open, и т.д.) */
    switch (extension){
        case "spc":
        case "nsf":
            return chiptune_gme_open_file(filepath, sample_rate);
            break;
        default:
            return chiptune_gme_open_file(filepath, sample_rate);
            break;
    }
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
