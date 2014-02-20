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

// solids
void red_cgen(struct rgb *out, int index) {
    rgb_init(out, 0xc0, 0x00, 0x00);
}
void green_cgen(struct rgb *out, int index) {
    rgb_init(out, 0x00, 0xc0, 0x00);
}
void blue_cgen(struct rgb *out, int index) {
    rgb_init(out, 0x00, 0x00, 0xc0);
}
void yellow_cgen(struct rgb *out, int index) {
    rgb_init(out, 0xc0, 0xc0, 0x00);
}
void orange_cgen(struct rgb *out, int index) {
    rgb_init(out, 0xc0, 0x40, 0x00);
}
void purple_cgen(struct rgb *out, int index) {
    rgb_init(out, 0xc0, 0x00, 0xc0);
}
void pink_cgen(struct rgb *out, int index) {
    rgb_init(out, 0xc0, 0x00, 0x40);
}
void cyan_cgen(struct rgb *out, int index) {
    rgb_init(out, 0x00, 0xc0, 0xc0);
}

// 2 color palletes
void red_orange_cgen(struct rgb *out, int index) {
    if (index <= 3) {
        red_cgen(out, index);
    } else {
        orange_cgen(out, index);
    }
}
void red_purple_cgen(struct rgb *out, int index) {
    if (index <= 3) {
        red_cgen(out, index);
    } else {
        purple_cgen(out, index);
    }
}
void red_blue_cgen(struct rgb *out, int index) {
    if (index <= 3) {
        red_cgen(out, index);
    } else {
        blue_cgen(out, index);
    }
}
void red_green_cgen(struct rgb *out, int index) {
    if (index <= 3) {
        red_cgen(out, index);
    } else {
        green_cgen(out, index);
    }
}

// linear between 2 primary
void red_to_green_cgen(struct rgb *out, int index) {
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
void blue_to_green_cgen(struct rgb *out, int index) {
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
void blue_to_red_cgen(struct rgb *out, int index) {
    if (index == 0) {
        rgb_init(out, 0x00, 0x00, 0xc0);
    } else if (index == 1) {
        rgb_init(out, 0x40, 0x00, 0xc0);
    } else if (index == 2) {
        rgb_init(out, 0x80, 0x00, 0xc0);
    } else if (index == 3) {
        rgb_init(out, 0xc0, 0x00, 0xc0);
    } else if (index == 4) {
        rgb_init(out, 0xc0, 0x00, 0x80);
    } else if (index == 5) {
        rgb_init(out, 0xc0, 0x00, 0x40);
    } else { 
        rgb_init(out, 0xc0, 0x00, 0x00);
    }
}

void rand_cgen(struct rgb *out, int index) {
    rgb_rand(out);
}

void (*cgens[])(struct rgb *out, int index) = {
    red_cgen,
    blue_cgen,
    green_cgen,
    yellow_cgen,
    orange_cgen,
    purple_cgen,
    pink_cgen,
    cyan_cgen,
    red_orange_cgen,
    red_purple_cgen,
    red_blue_cgen,
    red_green_cgen,
    red_to_green_cgen,
    blue_to_green_cgen,
    blue_to_red_cgen,
    rand_cgen
};

int cgens_len = sizeof(cgens)/sizeof(void *);

// MAIN EFFECTS
void all_on(struct visual_params *arg, struct rgb *out) {
    int color_index = rand() % 7;
    arg->color(out, color_index);
}

void some_off(struct visual_params *arg) {
    int i, j, color_index;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            int color_index = rand() % 7;
            if (prob(arg->energy)) {
                arg->color(&table[i][j], color_index);
            } else {
                rgb_init(&table[i][j], 0, 0, 0);
            }
        }
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

void strips_col(struct visual_params *arg) {
    int i, j;
    int color_index;
    for (j = 0; j < COLS; j++) {
        if (prob(arg->energy)) {
            color_index = rand() % 7;
            for (i = 0; i < ROWS_E; i++) {
                arg->color(&table[i][j], color_index); 
            }
        } else {
            for (i = 0; i < ROWS_E; i++) {
                rgb_init(&table[i][j], 0, 0, 0);    
            }
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

void positional_color_row(struct visual_params *arg) {
    int i, j;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            if (rgb_nz(&table[i][j])) {
                arg->color(&table[i][j], i);
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

void visuals_init() {
}

int iterations = 0;
void fill_table(int energy, int cgen) {
    struct visual_params vp;
    int i, j;
    
    int main_freq = 1;

    vp.color = cgens[cgen];
    vp.iterations = iterations;
    vp.energy = (int) energy;

    //strips_col(&vp);
    some_off(&vp);

    //rotate(rand()%4 * (flip()?-1:1), 0);
    //strip_col0(&vp);
    //positional_color_col(&vp);

    //strobe(&vp);
    //peaker(&vp);

    iterations++;
}
