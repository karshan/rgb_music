#ifndef RGB_H
#define RGB_H

#define COLS 32
#define ROWS 4
#define ROWS_E 7
#define COLS_P 4
#define ROWS_P 4
#define SQUARES COLS/4
#define BUFFER_SIZE (2*(ROWS*COLS*3 + SQUARES + 1))

struct rgb {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};
struct rgb table[ROWS_E][COLS*SQUARES];

extern int cgens_len;
extern int main_effects_len;

extern void visuals_init();
extern void fill_table(int energy, int cgen, int effect);

extern void rgb_music_init(void);
extern void rgb_music_iterate(unsigned int last_beat, unsigned int samples_per_beat, unsigned int sample_no, float freq);

extern inline unsigned char max2(unsigned char a, unsigned char b);
extern inline int flip();
extern inline int prob(int p);
extern inline void rgb_init(struct rgb *out, unsigned char r, unsigned char g, unsigned char b);
extern void rgb_rand(struct rgb *out);
extern inline int rgb_eq(struct rgb *a, struct rgb *b);
extern inline int rgb_nz(struct rgb *a);
extern int get_base_height(int col);
#endif
