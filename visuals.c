#include "rgb.h"

struct visual_params {
    int row;
    int col;
    int ht;
    void (*color)(struct rgb *out, int index);
    int energy;
    int iterations;
};

unsigned char rgb_clip(int in) {
    if (in < 0) return 0;
    else if (in > 255) return 255;
    else return (unsigned char)in;
}

void rand_cgen(struct rgb *out, int index) {
    rgb_rand(out);
}

void red_yellow_cgen(struct rgb *out, int index) {
    if (index == 0) {
        rgb_init(out, 0xc0, 0xc0, 0);
    } else if (index == 1) {
        rgb_init(out, 0xc0, 0x80, 0);
    } else if (index == 2) {
        rgb_init(out, 0xc0, 0x40, 0);
    } else if (index == 3) {
        rgb_init(out, 0xc0, 0x00, 0);
    }
}

// MAIN EFFECTS
void some_off(struct visual_params *arg, struct rgb *out) {
    if (prob(arg->energy)) {
        arg->color(out, 0);
    } else {
        rgb_init(out, 0, 0, 0);
    }
}

void histogram(struct visual_params *arg, struct rgb *out) {
    out->r = 0; out->b = 0; out->g = 0;
    if (arg->row > 0) {
        if (rgb_nz(&table[arg->row - 1][arg->col])) {
            if (prob(arg->energy)) {
                arg->color(out, arg->ht/2);
            }
        }
    } else {
        if (arg->ht == 0) {
            if (prob(66)) {
                arg->color(out, arg->ht/2);
            }
        } else {
            if (prob(33)) {
                arg->color(out, arg->ht/2);
            }
        }
    }
}

// FILTERS
void strobe(struct visual_params *arg) {
    int i, j;
    if (arg->iterations % 2 == 1) {
        for (i = 0; i < ROWS; i++) {
            for (j = 0; j < COLS; j++) {
                rgb_init(&(table[i][j]), 0, 0, 0);
            }
        }
    }
}

void peaker(struct visual_params *arg) {
    int i, j;
    for (i = 0; i < ROWS - 1; i++) {
        for (j = 0; j < COLS; j++) {
            if (rgb_nz(&table[i + 1][j])) {
                rgb_init(&(table[i][j]), 0, 0, 0);
            }
        }
    }
}

int iterations = 0;
void fill_table() {
    struct visual_params vp;
    int i, j;

    vp.color = red_yellow_cgen;
    vp.iterations = iterations;
    vp.energy = 50;

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            vp.row = i;
            vp.col = j;
            vp.ht = get_base_height(j) + i;

            histogram(&vp, &(table[i][j]));
        }
    }

    //strobe(&vp);
    peaker(&vp);

    iterations++;
}
