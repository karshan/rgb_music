#include "rgb.h"

struct visual_params {
    int row;
    int col;
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

void purple_cgen(struct rgb *out, int index) {
    rgb_init(out, 0xc0, 0x00, 0xc0);
}

void green_cgen(struct rgb *out, int index) {
    rgb_init(out, 0x00, 0xc0, 0x00);
}

void red_yellow_cgen(struct rgb *out, int index) {
    if (index == 0) {
        rgb_init(out, 0xc0, 0x00, 0x00);
    } else if (index == 1) {
        rgb_init(out, 0xc0, 0x40, 0x00);
    } else if (index == 2) {
        rgb_init(out, 0xc0, 0x80, 0x00);
    } else if (index == 3) {
        rgb_init(out, 0xc0, 0xc0, 0x00);
    } else if (index == 4) {
        rgb_init(out, 0x80, 0xc0, 0x00);
    } else if (index == 5) {
        rgb_init(out, 0x40, 0xc0, 0x00);
    } else { 
        rgb_init(out, 0x00, 0xc0, 0x00);
    }
}

void blue_cyan_cgen(struct rgb *out, int index) {
    if (index == 0) {
        rgb_init(out, 0x00, 0x00, 0xc0);
    } else if (index == 1) {
        rgb_init(out, 0x00, 0x40, 0xc0);
    } else if (index == 2) {
        rgb_init(out, 0x00, 0x80, 0xc0);
    } else if (index == 3) {
        rgb_init(out, 0x00, 0xc0, 0xc0);
    } else if (index == 4) {
        rgb_init(out, 0x00, 0xc0, 0x80);
    } else if (index == 5) {
        rgb_init(out, 0x00, 0xc0, 0x40);
    } else { 
        rgb_init(out, 0x00, 0xc0, 0x00);
    }
}



// MAIN EFFECTS
void all_on(struct visual_params *arg, struct rgb *out) {
    int color_index = rand() % 7;
    arg->color(out, color_index);
}

void some_off(struct visual_params *arg, struct rgb *out) {
    int color_index = rand() % 7;
    if (prob(arg->energy)) {
        arg->color(out, color_index);
    } else {
        rgb_init(out, 0, 0, 0);
    }
}

void histogram(struct visual_params *arg, struct rgb *out) {
    out->r = 0; out->b = 0; out->g = 0;
    if (arg->row > 0) {
        if (rgb_nz(&table[arg->row - 1][arg->col])) {
            if (prob(arg->energy)) {
                arg->color(out, arg->row);
            }
        }
    } else {
        if (prob(arg->energy)) {
            arg->color(out, arg->row);
        }
    }
}

void strips(struct visual_params *arg, struct rgb *out) {
    int color_index = arg->row;
    out->r = 0; out->b = 0; out->g = 0;
    if (arg->row > 0) {
        if (rgb_nz(&table[arg->row - 1][arg->col])) {
            arg->color(out, color_index);
        }
    } else {
        if (prob(arg->energy)) {
            arg->color(out, color_index);
        }
    }
}

// FILTERS
void strobe(struct visual_params *arg) {
    int i, j;
    if (arg->iterations % 2 == 1) {
        for (i = 0; i < ROWS_E; i++) {
            for (j = 0; j < COLS; j++) {
                rgb_init(&(table[i][j]), 0, 0, 0);
            }
        }
    }
}

void peaker(struct visual_params *arg) {
    int i, j;
    for (i = 0; i < ROWS_E - 1; i++) {
        for (j = 0; j < COLS; j++) {
            if (rgb_nz(&table[i + 1][j])) {
                rgb_init(&(table[i][j]), 0, 0, 0);
            }
        }
    }
}

struct rgb positional_color_table[ROWS_E][COLS];
void positional_color(struct visual_params *arg) {
    int i, j;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            if (rgb_nz(&table[i][j])) {
                table[i][j] = positional_color_table[i][j];
            }
        }
    }
}

void positional_color_col(struct visual_params *arg) {
    int i, j;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            if (rgb_nz(&table[i][j])) {
                arg->color(&table[i][j], ((float)j/COLS)*7);
            }
        }
    }
}

void strip_col0(struct visual_params *arg) {
    int i;
    if (prob(arg->energy)) {
        for (i = 0; i < ROWS_E; i++) {
            arg->color(&table[i][0], i);
        }
    }
}

void translate(int x, int y) {
    struct rgb buffer[ROWS_E][COLS];
    int i, j;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            if (i - y >= 0 && i - y < ROWS_E && j - x >= 0 && j - x < COLS) {
                buffer[i][j] = table[i - y][j - x];
            } else {
                rgb_init(&buffer[i][j], 0, 0, 0);
            }
        }
    }

    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            table[i][j] = buffer[i][j];
        }
    }
}

void drag(int x, int y) {
    struct rgb buffer[ROWS_E][COLS];
    int i, j;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            if (i - y >= 0 && i - y < ROWS_E && j - x >= 0 && j - x < COLS) {
                buffer[i][j] = table[i - y][j - x];
            } else {
                buffer[i][j] = table[i][j];
            }
        }
    }

    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            table[i][j] = buffer[i][j];
        }
    }
}



inline int real_mod(int a, int m) {
    if (a < 0) {
        return (a + m) % m;
    } else {
        return a % m;
    }
}

void rotate(int x, int y) {
    struct rgb buffer[ROWS_E][COLS];
    int i, j;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            buffer[i][j] = table[real_mod((i - y), ROWS_E)][real_mod((j - x), COLS)];
        }
    }

    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            table[i][j] = buffer[i][j];
        }
    }
}

void positional_color_init() {
    int i, j;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            red_yellow_cgen(&positional_color_table[i][j], get_base_height(j) + i);
        }
    }
}


void visuals_init() {
    positional_color_init();
}

int iterations = 0;
void fill_table(int energy) {
    struct visual_params vp;
    int i, j;
    
    int main_freq = 1;

    vp.color = blue_cyan_cgen;
    vp.iterations = iterations;
    vp.energy = (int) energy;

    if (iterations % main_freq == 0) {
        for (i = 0; i < ROWS_E; i++) {
            for (j = 0; j < COLS; j++) {
                vp.row = i;
                vp.col = j;

                //strips(&vp, &(table[i][j]));
                //histogram(&vp, &(table[i][j]));
                //some_off(&vp, &(table[i][j]));
            }
        }
    }
    drag(1, 0);
    strip_col0(&vp);
    positional_color_col(&vp);

    //strobe(&vp);
    //peaker(&vp);

    iterations++;
}
