#include <stdio.h>
#include "rgb.h"

int main() {
    rgb_music_init();

    while(1) {
        rgb_music_iterate(5, 1, 5, 200);
        getchar();
    }
}
