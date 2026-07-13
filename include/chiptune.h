#ifndef CHIPTUNE_H
#define CHIPTUNE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Непрозрачный указатель на экземпляр плеера */
typedef struct ChiptunePlayer ChiptunePlayer;

/* Информация о треке */
typedef struct {
    char title[256];
    char author[256];
    char game[256];
    char comment[512];
    int tracks_count;
    int current_track;
    int duration_ms;
} ChiptuneInfo;

/* Жизненный цикл */
ChiptunePlayer* chiptune_open_file(const char* filepath, long sample_rate);
ChiptunePlayer* chiptune_open_memory(const void* data, size_t size, const char* ext, long sample_rate);
void chiptune_close(ChiptunePlayer* player);

/* Управление воспроизведением */
int chiptune_start_track(ChiptunePlayer* player, int track_index);
void chiptune_set_looping(ChiptunePlayer* player, int enable);
int chiptune_seek(ChiptunePlayer* player, int position_ms);
int chiptune_tell(const ChiptunePlayer* player);

/* Метаданные и состояние */
int chiptune_get_info(const ChiptunePlayer* player, ChiptuneInfo* info);

/* Рендеринг (заполнение интерливинг стерео-буфера short: L, R, L, R...) */
int chiptune_play(ChiptunePlayer* player, short* buffer, int max_samples);

#ifdef __cplusplus
}
#endif

#endif /* CHIPTUNE_H */
