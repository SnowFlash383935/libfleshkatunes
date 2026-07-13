#include <stdio.h>
#include <stdlib.h>
#include "gme_helper.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_tune> <output_wav>\n", argv[0]);
        return 1;
    }

    printf("Rendering %s -> %s\n", argv[1], argv[2]);
    // Рендерим 3 минуты (180 секунд)
    if (render_gme_to_wav(argv[1], argv[2], 180) != 0) {
        fprintf(stderr, "Failed to render track.\n");
        return 1;
    }

    return 0;
}
