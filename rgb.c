#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#define COLS 16
#define ROWS 4
#define COLS_P 4
#define ROWS_P 4
#define SQUARES 4
#define BUFFER_SIZE (2*(ROWS*COLS*3 + SQUARES + 1))

struct rgb {
    char r;
    char g;
    char b;
};

struct rgb table[ROWS][COLS*SQUARES];
unsigned char square_ids[SQUARES] = { 5, 7, 6, 3 };

int usb_fd;

char buffer[BUFFER_SIZE];
void add_color(char *buffer, int *p, int *nibble, char color) {
    if (nibble == 0) {
        buffer[*p++] = 0x1;
        buffer[*p] = (color & 0xf0) >> 4;
    } else {
        buffer[*p] |= (color & 0xf0);
    }
    *nibble = !*nibble;
}
void draw_table(void) {
    int i, j, k, p = 0, nibble = 0;
    for (i = 0; i < SQUARES; i++) {
        buffer[p++] = 0xf0;
        buffer[p++] = square_ids[i];
        for (j = 0; j < ROWS_P; j++) {
            for (k = 0; k < COLS_P; k++) {
                add_color(buffer, &p, &nibble, table[j][k + COLS_P*i].r);
                add_color(buffer, &p, &nibble, table[j][k + COLS_P*i].b);
                add_color(buffer, &p, &nibble, table[j][k + COLS_P*i].g);
            }
        }
    }
    buffer[p++] = 0xf0;
    buffer[p++] = 0xf0;
    write(usb_fd, buffer, BUFFER_SIZE);
}

unsigned int iterations_per_beat = 2;
unsigned int iterations = 0;
unsigned int last_iteration = 0;
void by_pitch(int row, int col, float freq, struct rgb *out) {
    out->r = 0;
    out->b = 0;
    out->g = 0;
    if (iterations % 2 == 0 && rand()%2 == 0) {
        out->g = 255;
    }
    return;

    if (rand()%2 == 0) {
        if (rand()%2 == 0) {
            out->g = 255;
        } else {
            out->r = 255;
            out->b = 128;
        }
    }
}
 

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
        
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            by_pitch(i, j, freq, &(table[i][j]));
        }
    }

    draw_table();
    iterations++;
}
