#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

#define COLS 12
#define ROWS 4
#define COLS_P 4
#define ROWS_P 4
#define SQUARES 3
#define BUFFER_SIZE (2*(ROWS*COLS*3 + SQUARES))

struct rgb {
    char r;
    char g;
    char b;
};

struct rgb table[ROWS][COLS*SQUARES];
unsigned char square_ids[SQUARES] = { 0x5, 0x0, 0x2a };

int usb_fd;

char buffer[BUFFER_SIZE];
void draw_table(void) {
    int i, j, k, p = 0;
    for (i = 0; i < SQUARES; i++) {
        buffer[p++] = 0xf0;
        buffer[p++] = square_ids[i];
        for (j = 0; j < ROWS_P; j++) {
            for (k = 0; k < COLS_P; k++) {
                buffer[p++] = 0x1;
                buffer[p++] = table[j][k + COLS_P*i].r;
                buffer[p++] = 0x1;
                buffer[p++] = table[j][k + COLS_P*i].b;
                buffer[p++] = 0x1;
                buffer[p++] = table[j][k + COLS_P*i].g;
            }
        }
    }
    write(usb_fd, buffer, BUFFER_SIZE);
}

void greenpink(struct rgb *out) {
    out->r = 0;
    out->b = 0;
    out->g = 0;
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
    srand(time(0));
}
void rgb_iterate(void) {
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            greenpink(&(table[i][j]));
        }
    }

    draw_table();
}
