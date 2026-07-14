#include "chiptune_internal.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>

/* Объявление фабричной функции GME-бэкенда */
ChiptunePlayer* chiptune_gme_open_file(const char* filepath, long sample_rate);

/* Кроссплатформенный аналог регистронезависимого сравнения (если под Windows, используется _stricmp) */
#if defined(_MSC_VER)
#define strcasecmp _stricmp
#endif

ChiptunePlayer* chiptune_open_file(const char* filepath, long sample_rate) {
    if (!filepath) return NULL;

    /* Ищем последнее вхождение точки в путь к файлу */
    const char *dot = strrchr(filepath, '.');
    
    /* Защита от отсутствия расширения или если точка стоит в самом начале (например, скрытый файл) */
    if (!dot || dot == filepath || *(dot + 1) == '\0') {
        /* Возвращаем поведение по умолчанию (дефолтный бэкенд) */
        return chiptune_gme_open_file(filepath, sample_rate);
    }

    const char *extension = dot + 1;

    /* Диспетчеризация форматов на базе GME */
    if (strcasecmp(extension, "spc") == 0 ||
        strcasecmp(extension, "nsf") == 0 ||
        strcasecmp(extension, "vgm") == 0 ||
        strcasecmp(extension, "gym") == 0 ||
        strcasecmp(extension, "hes") == 0 ||
        strcasecmp(extension, "kss") == 0 ||
        strcasecmp(extension, "nsfe") == 0 ||
        strcasecmp(extension, "sap") == 0 ||
        strcasecmp(extension, "spc") == 0) {
        return chiptune_gme_open_file(filepath, sample_rate);
    }

    /* Подготовка к будущим расширениям семейства PSF (mini-usf, mini-gsf и т.д.) */
    /*
    if (strcasecmp(extension, "miniusf") == 0 || strcasecmp(extension, "minigsf") == 0) {
        // Здесь в будущем будет вызов utilities.h: psf_get_lib_name()
        // и последующая склейка путей перед передачей в профильный бэкенд
    }
    */

    /* Фолбэк для всех прочих неподтвержденных расширений */
    return chiptune_gme_open_file(filepath, sample_rate);
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
