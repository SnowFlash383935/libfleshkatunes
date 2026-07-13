#define _ISOC99_SOURCE
#include "chiptune_internal.h"
#include <gme/gme.h>
#include <stdlib.h>
#include <string.h>

static void gme_error_check(const char* err) {
    if (err) {
        /* Обработка ошибки */
    }
}

static int chiptune_gme_start_track(ChiptunePlayer* p, int track) {
    Music_Emu* emu = (Music_Emu*)p->backend_data;
    if (gme_start_track(emu, track) != NULL) return -1;
    p->current_track = track;
    /* Применяем состояние зацикливания при старте трека */
    gme_set_autoload_playback_limit(emu, p->is_looping ? 0 : 1);
    return 0;
}

static void chiptune_gme_set_looping(ChiptunePlayer* p, int enable) {
    p->is_looping = enable;
    Music_Emu* emu = (Music_Emu*)p->backend_data;
    if (emu) {
        gme_set_autoload_playback_limit(emu, enable ? 0 : 1);
        gme_ignore_silence(emu, enable ? 1 : 0);
    }
}

static int chiptune_gme_seek(ChiptunePlayer* p, int pos_ms) {
    Music_Emu* emu = (Music_Emu*)p->backend_data;
    return gme_seek(emu, pos_ms) == NULL ? 0 : -1;
}

static int chiptune_gme_tell(const ChiptunePlayer* p) {
    Music_Emu* emu = (Music_Emu*)p->backend_data;
    return (int)gme_tell(emu);
}

static int chiptune_gme_get_info(const ChiptunePlayer* p, ChiptuneInfo* info) {
    Music_Emu* emu = (Music_Emu*)p->backend_data;
    gme_info_t* ginfo = NULL;
    if (gme_track_info(emu, &ginfo, p->current_track) != NULL) return -1;

    strncpy(info->title, ginfo->song, sizeof(info->title) - 1);
    strncpy(info->author, ginfo->author, sizeof(info->author) - 1);
    strncpy(info->game, ginfo->game, sizeof(info->game) - 1);
    strncpy(info->comment, ginfo->comment, sizeof(info->comment) - 1);
    info->tracks_count = gme_track_count(emu);
    info->current_track = p->current_track;
    info->duration_ms = ginfo->length > 0 ? (int)ginfo->length : (int)ginfo->play_length;

    gme_free_info(ginfo);
    return 0;
}

static int chiptune_gme_play(ChiptunePlayer* p, short* buf, int max_samples) {
    Music_Emu* emu = (Music_Emu*)p->backend_data;
    /* gme_play принимает количество сэмплов (не фреймов) */
    if (gme_play(emu, max_samples, buf) != NULL) return -1;
    return max_samples;
}

static void chiptune_gme_close(ChiptunePlayer* p) {
    Music_Emu* emu = (Music_Emu*)p->backend_data;
    if (emu) {
        gme_delete(emu);
    }
    free(p);
}

/* Функция фабрики для открытия файла через GME */
ChiptunePlayer* chiptune_gme_open_file(const char* filepath, long sample_rate) {
    Music_Emu* emu = NULL;
    if (gme_open_file(filepath, &emu, sample_rate) != NULL) return NULL;

    ChiptunePlayer* p = (ChiptunePlayer*)malloc(sizeof(ChiptunePlayer));
    if (!p) {
        gme_delete(emu);
        return NULL;
    }

    p->sample_rate = sample_rate;
    p->is_looping = 1; /* По умолчанию зациклено */
    p->current_track = 0;
    p->backend_data = emu;

    p->start_track = chiptune_gme_start_track;
    p->set_looping = chiptune_gme_set_looping;
    p->seek = chiptune_gme_seek;
    p->tell = chiptune_gme_tell;
    p->get_info = chiptune_gme_get_info;
    p->play = chiptune_gme_play;
    p->close = chiptune_gme_close;

    /* Инициализируем первый трек */
    chiptune_gme_start_track(p, 0);
    chiptune_gme_set_looping(p, 1);

    return p;
}
