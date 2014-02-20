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

void blue_cyan_cgen(struct rgb *out, int index) {
    index = rand() % 7;
    if (index == 0) {
        rgb_init(out, 0x00, 0xc0, 0xc0);
    } else if (index == 1) {
        rgb_init(out, 0x00, 0x80, 0xc0);
    } else if (index == 2) {
        rgb_init(out, 0x00, 0x40, 0xc0);
    } else if (index == 3) {
        rgb_init(out, 0x00, 0x00, 0xc0);
    } else if (index == 4) {
        rgb_init(out, 0x00, 0xc0, 0x80);
    } else if (index == 5) {
        rgb_init(out, 0x00, 0xc0, 0x40);
    } else if (index == 6) {
        rgb_init(out, 0x00, 0xc0, 0x00);
    }
}



// MAIN EFFECTS
void all_on(struct visual_params *arg, struct rgb *out) {
    arg->color(out, rand() % 4);
}

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

void strips(struct visual_params *arg, struct rgb *out) {
    out->r = 0; out->b = 0; out->g = 0;
    if (arg->row > 0) {
        if (rgb_nz(&table[arg->row - 1][arg->col])) {
            arg->color(out, 0);
        }
    } else {
        if (prob(arg->energy)) {
            arg->color(out, 0);
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

struct rgb positional_color_table[ROWS][COLS];
void positional_color(struct visual_params *arg) {
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (rgb_nz(&table[i + 1][j])) {
                table[i][j] = positional_color_table[i][j];
            }
        }
    }
}

void translate(int x, int y) {
    struct rgb buffer[ROWS][COLS];
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (i - y > 0 && i - y < ROWS && j - x > 0 && j - x < COLS) {
                buffer[i][j] = table[i - y][j - x];
            } else {
                rgb_init(&buffer[i][j], 0, 0, 0);
            }
        }
    }

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            table[i][j] = buffer[i][j];
        }
    }
}

void visuals_init() {
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            rgb_rand(&positional_color_table[i][j]);
        }
    }
}

int iterations = 0;
void fill_table(float energy) {
    struct visual_params vp;
    int i, j;

    vp.color = blue_cyan_cgen;
    vp.iterations = iterations;
    vp.energy = (int) energy;

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            vp.row = i;
            vp.col = j;
            vp.ht = get_base_height(j) + i;

            some_off(&vp, &(table[i][j]));
        }
    }

    //strobe(&vp);
    //peaker(&vp);

    iterations++;
}
