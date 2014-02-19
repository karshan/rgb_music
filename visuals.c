#include "rgb.h"

struct visual_params {
    int row;
    int col;
    struct rgb color;
    int iterations;
};

unsigned char max2(char a, char b) {
    return a > b ? a : b;
}

void init_color(struct rgb *out, char r, char g, char b) {
    out->r = r; out->g = g; out->b = b;
}

void init_rand_color(struct rgb *out) {
    unsigned char max, r, g, b;
    r = (rand()%8) << 5;
    g = (rand()%8) << 5;
    b = (rand()%8) << 5;
    max = max2(max2(r, g), b);
    max = (max == 0) ? 1 : max;
    init_color(out, r*255/max, g*255/max, b*255/max);
}

void gc_random_strobe(struct visual_params *arg, struct rgb *out) {
    out->r = 0; out->b = 0; out->g = 0;
    if (arg->iterations % 2 == 0 && rand()%2 == 0) {
        init_rand_color(out);
        //*out = arg->color;
    }
}

void gc_random_strobe_sym(struct visual_params *arg, struct rgb *out) {
    out->r = 0; out->b = 0; out->g = 0;
    if (arg->col % 4 >= 2) {
        *out = table[arg->row][arg->col % 2];
        return;
    }
    if (arg->iterations % 2 == 0 && rand()%2 == 0) {
        init_rand_color(out);
        //*out = arg->color;
    }
}

void gc_hist(struct visual_params *arg, struct rgb *out) {
    out->r = 0; out->b = 0; out->g = 0;
    if (arg->row > 0) {
        if (memcmp(&table[arg->row - 1][arg->col], &arg->color, sizeof(struct rgb)) == 0 && rand() % 2 == 0) {
            *out = arg->color;
        }
    } else if (rand() % 2 == 0) {
        *out = arg->color;
    }
}


void gc_count(struct visual_params *arg, struct rgb *out) {
    out->r = 0; out->b = 0; out->g = 0;
    if (arg->iterations & (1 << (arg->col + arg->row*COLS)))
        *out = arg->color;
}

int iterations = 0;
void fill_table() {
    struct visual_params vp;
    int i, j;

    init_color(&vp.color, 255, 0, 0);
    vp.iterations = iterations;

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            vp.row = i;
            vp.col = j;
            gc_hist(&vp, &(table[i][j]));
        }
    }

    iterations++;
}


