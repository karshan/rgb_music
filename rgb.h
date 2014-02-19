#ifndef RGB_H
#define RGB_H

#define COLS 32
#define ROWS 4
#define COLS_P 4
#define ROWS_P 4
#define SQUARES 8
#define BUFFER_SIZE (2*(ROWS*COLS*3 + SQUARES + 1))

struct rgb {
    char r;
    char g;
    char b;
};
struct rgb table[ROWS][COLS*SQUARES];

extern void fill_table();
extern void rgb_init(void);
extern void rgb_iterate(unsigned int last_beat, unsigned int samples_per_beat, unsigned int sample_no, float freq);

#endif
