#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "rgb.h"

unsigned char square_ids[SQUARES] = { 6, 1, 2, 3, 4, 5, 7, 8 };

int usb_fd;

char buffer[BUFFER_SIZE];
void add_color(char *buffer, int *p, int *nibble, char color) {
    if (*nibble == 0) {
        buffer[(*p)++] = 0x1;
        buffer[*p] = (color & 0xf0) >> 4;
    } else {
        buffer[(*p)++] |= (color & 0xf0);
    }
    *nibble = !(*nibble);
}
void draw_table(void) {
    int i, j, k, p = 0, nibble = 0;
    for (i = 0; i < SQUARES; i++) {
        buffer[p++] = 0x80 | square_ids[i];
        for (j = 0; j < ROWS_P; j++) {
            for (k = 0; k < COLS_P; k++) {
                buffer[p++] = ((table[j][k + COLS_P*i].g & 0xc0 >> 2) | ((table[j][k + COLS_P*i].b & 0xc0) >> 4) | ((table[j][k + COLS_P*i].r & 0xc0) >> 6)) & 0x7f;
            }
        }
    }
    buffer[p++] = 0xf0;
    
    write(usb_fd, buffer, p);
}

unsigned int iterations_per_beat = 2;
unsigned int last_iteration = 0;


void rgb_init(void) {
    usb_fd = open("/dev/hidraw0", O_WRONLY);
    if (usb_fd < 0) {
        usb_fd = open("/dev/hidraw1", O_WRONLY);
    }
    if (usb_fd < 0) {
        printf("FAILED TO OPEN /dev/hidraw{0,1}\n");
        exit(-1);
    }

    srand(time(0));
}

void rgb_iterate(unsigned int last_beat, unsigned int samples_per_beat, unsigned int sample_no, float freq) {
    int i, j;

    if (last_beat != sample_no) { //sample_no < last_iteration + samples_per_beat/iterations_per_beat) {
        return;
    }

    last_iteration = sample_no;
        
    fill_table();
    draw_table();
}
