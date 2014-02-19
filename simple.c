#include <stdio.h>
#include "rgb.h"

int main() {
    rgb_init();

    while(1) {
        rgb_iterate(5, 1, 5, 200);
        getchar();
    }
}
