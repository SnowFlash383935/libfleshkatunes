#ifndef CHIPTUNE_INTERNAL_H
#define CHIPTUNE_INTERNAL_H

#include "chiptune.h"

struct ChiptunePlayer {
    long sample_rate;
    int is_looping;
    int current_track;
    void* backend_data; /* Здесь хранится Music_Emu* для GME или другие контексты */
    
    /* Виртуальная таблица методов бэкенда */
    int (*start_track)(ChiptunePlayer* p, int track);
    void (*set_looping)(ChiptunePlayer* p, int enable);
    int (*seek)(ChiptunePlayer* p, int pos_ms);
    int (*tell)(const ChiptunePlayer* p);
    int (*get_info)(const ChiptunePlayer* p, ChiptuneInfo* info);
    int (*play)(ChiptunePlayer* p, short* buf, int max_samples);
    void (*close)(ChiptunePlayer* p);
};

#endif
