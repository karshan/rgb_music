#include "rgb.h"

unsigned char rgb_clip(int in) {
    if (in < 0) return 0;
    else if (in > 255) return 255;
    else return (unsigned char)in;
}

void clear_table() {
    int i, j;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            rgb_init(&table[i][j], 0, 0, 0);
        }
    }
}

void draw_frame(struct visual_params *arg, char frame[4][4], int p) {
    int s, i, j, col;
    struct rgb *out;
    clear_table();
    for (s = 0; s < SQUARES; s++) {
        if (prob(p)) {
            for (i = 0; i < ROWS_P; i++) {
                for (j = 0; j < COLS_P; j++) {
                    col = j + COLS_P*s;
                    out = &table[i + get_base_height(col)][col];
                    if (frame[i][j]) {
                        arg->color(out, frame[i][j]);
                    } else {
                        rgb_init(out, 0, 0, 0);
                    }
                }
            }
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

void strip_col_x(struct visual_params *arg, int x) {
    int i;
    for (i = 0; i < ROWS_E; i++) {
        arg->color(&table[i][x], i);
    }
}

void clear_col_x(struct visual_params *arg, int x) {
    int i;
    for (i = 0; i < ROWS_E; i++) {
        rgb_init(&table[i][x], 0, 0, 0);
    }
}

void strip_row_x(struct visual_params *arg, int x) {
    int i;
    for (i = 0; i < COLS; i++) {
        arg->color(&table[x][i], ((float)i/COLS)*7);
    }
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
void green_pink_cgen(struct rgb *out, int index) {
    if (index <= 3) {
        green_cgen(out, index);
    } else {
        pink_cgen(out, index);
    }
}
void purple_cyan_cgen(struct rgb *out, int index) {
    if (index <= 3) {
        purple_cgen(out, index);
    } else {
        cyan_cgen(out, index);
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
    green_pink_cgen,
    purple_cyan_cgen,
    red_to_green_cgen,
    blue_to_green_cgen,
    blue_to_red_cgen,
    rand_cgen
};

int cgens_len = sizeof(cgens)/sizeof(void *);

// MAIN EFFECTS
void all_on(struct visual_params *arg) {
    int i, j, color_index;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            int color_index = rand() % 7;
            arg->color(&table[i][j], color_index);
        }
    }
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

void histogram(struct visual_params *arg) {
    int i, j, color_index;
    clear_table();
    for (j = 0; j < COLS; j++) {
        for (i = 0; i < ROWS_E; i++) {
            if (prob(arg->energy)) {
                arg->color(&table[i][j], i);
            } else {
                break;
            }
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

void strips_diag(struct visual_params *arg) {
    int i, j, color_index;
    clear_table();
    for (j = -4; j < COLS + 4; j++) {
        if (prob(arg->energy)) {
            color_index = rand() % 7;
            for (i = 0; i < ROWS_E; i++) {
                if (arg->dir) {
                    if (j + i < COLS && j + i >= 0) {
                        arg->color(&table[i][j + i], color_index);
                    }
                } else {
                    if (j + i < COLS && j + i >= 0) {
                        arg->color(&table[i][j - i], color_index);
                    }
                }
            }
        }
    }
}

void in_and_out(struct visual_params *arg) {
    char f1[4][4] = {
        {0,0,0,0},
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0}
    };
    char f2[4][4] = {
        {4,4,4,4},
        {4,0,0,4},
        {4,0,0,4},
        {4,4,4,4}
    };
    if ((arg->iterations % 2) == 0) {
        draw_frame(arg, f1, arg->energy);
    } else {
        draw_frame(arg, f2, arg->energy);
    }
}

void x_and_o(struct visual_params *arg) {
    char f1[4][4] = {
        {1,0,0,1},
        {0,1,1,0},
        {0,1,1,0},
        {1,0,0,1}
    };
    char f2[4][4] = {
        {0,4,4,0},
        {4,0,0,4},
        {4,0,0,4},
        {0,4,4,0}
    };
    if ((arg->iterations % 2) == 0) {
        draw_frame(arg, f1, arg->energy);
    } else {
        draw_frame(arg, f2, arg->energy);
    }
}

void raindrops(struct visual_params *arg) {
    int i;
    translate(arg->dir ? -1 : 1, 0);
    for (i = 0; i < ROWS_E; i++) {
        if (prob(arg->energy)) {
            arg->color(&table[i][arg->dir ? COLS - 1: 0], rand() % 7);
        }
    }
}

void squares(struct visual_params *arg) {
    char frame[4][4];
    int color_index = rand() % 7;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            frame[i][j] = color_index;
        }
    }
    draw_frame(arg, frame, arg->energy);
}

void spiral(struct visual_params *arg) {
}

void fill_col(struct visual_params *arg) {
    /*int i, j;
    char is_full = 1;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            if (!rgb_nz(&table[i][j])) {
                is_full = 0;
            }
        }
    }
    if (is_full) { clear_table(); return; }*/
    drag(arg->dir ? -1 : 1, 0);
    strip_col_x(arg, arg->dir ? COLS - 1 : 0);
}

void unfill_col(struct visual_params *arg) {
    /*int i, j;
    char is_full = 1;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            if (!rgb_nz(&table[i][j])) {
                is_full = 0;
            }
        }
    }
    if (is_full) { clear_table(); return; }*/
    drag(arg->dir ? -1 : 1, 0);
    clear_col_x(arg, arg->dir ? COLS - 1 : 0);
}



void fill_row(struct visual_params *arg) {
    int i, j;
    char is_full = 1;
    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            if (!rgb_nz(&table[i][j])) {
                is_full = 0;
            }
        }
    }
    if (is_full) { clear_table(); return; }
    drag(0, arg->dir ? -1 : 1);
    strip_row_x(arg, arg->dir ? ROWS_E - 1 : 0);
}

void fill_spiral(struct visual_params *arg) {
}
 
void (*main_effects[])(struct visual_params *out) = {
    all_on,
    some_off,
    histogram,
    strips_col,
    strips_diag,
    squares,
    in_and_out,
    x_and_o,
    raindrops,
    fill_col,
    unfill_col,
    fill_row,
};

int main_effects_len = sizeof(main_effects)/sizeof(void*);

struct song songs[] = {
    { 
        .vps = {
            {
                .color = red_orange_cgen,
                .energy = 0,
                .iterations = 0,
                .dir = 0,
                .multiplier = 4
            }, 
        },
        .effects = {
            some_off,
        },
        .effects_len = 1,
        .trans_on = 0,
    }, 
    { 
        .vps = {
            {
                .color = purple_cyan_cgen,
                .energy = 0,
                .iterations = 0,
                .dir = 0,
                .multiplier = 4
            }, {
                .color = blue_to_green_cgen,
                .energy = 30,
                .iterations = 0,
                .dir = 0,
                .multiplier = 8
            }
        },
        .effects = {
            in_and_out,
            some_off,
        },
        .effects_len = 2,
        .trans_on = 0,
    }, { 
        .vps = {
            {
                .color = red_to_green_cgen,
                .energy = 45,
                .iterations = 0,
                .dir = 0,
                .multiplier = 4
            }, {
                .color = red_orange_cgen,
                .energy = 10,
                .iterations = 0,
                .dir = 0,
                .multiplier = 8
            }
        },
        .effects = {
            histogram,
            strips_col,
        },
        .effects_len = 2,
        .trans_on = 0,
    },
};

int songs_len = sizeof(songs)/sizeof(struct song);

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
void (*trans_effect)(struct visual_params *arg) = 0;
int bak_multiplier = 1;
extern int song_no, effect_no;
void do_transition() {
    songs[song_no].trans_start = iterations;
    songs[song_no].trans_on = 1;
    bak_multiplier = songs[song_no].vps[effect_no].multiplier;
    songs[song_no].vps[effect_no].multiplier = 1;
}

void fill_table() {
    struct visual_params *vp = &songs[song_no].vps[effect_no];

    vp->iterations = iterations;

    if (songs[song_no].trans_on == 1) {
        if ((iterations - songs[song_no].trans_start) >= 1) {
            songs[song_no].vps[effect_no].multiplier = 32;
            trans_effect = fill_col;
            songs[song_no].trans_on = 2;
            songs[song_no].trans_start = iterations;
        }
    } else if (songs[song_no].trans_on == 2) {
        if (iterations - songs[song_no].trans_start == COLS) {
            songs[song_no].vps[effect_no].dir = !songs[song_no].vps[effect_no].dir;
            trans_effect = unfill_col;
        } else if (iterations - songs[song_no].trans_start >= COLS*2) {
            trans_effect = 0;
            songs[song_no].vps[effect_no].multiplier = bak_multiplier;
            songs[song_no].trans_on = 0;
            effect_no++;
        }
    }

    if (trans_effect == 0) {
        songs[song_no].effects[effect_no](vp);
    } else {
        trans_effect(vp); 
    }
    
    //rotate(rand()%4 * (flip()?-1:1), 0);
    //strip_col0(&vp);
    //positional_color_col(&vp);

    //strobe(&vp);
    //peaker(&vp);

    iterations++;
}
