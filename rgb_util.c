#include "rgb.h"

inline unsigned char max2(unsigned char a, unsigned char b) {
    return a > b ? a : b;
}

inline int flip() {
    return rand() % 2;
}

inline int prob(int p) {
    return rand() % 100 < p;
}

inline void rgb_init(struct rgb *out, unsigned char r, unsigned char g, unsigned char b) {
    out->r = r; out->g = g; out->b = b;
}

void rgb_rand(struct rgb *out) {
    unsigned char max, r, g, b;
    r = (rand()%8) << 5;
    g = (rand()%8) << 5;
    b = (rand()%8) << 5;
    max = max2(max2(r, g), b);
    max = (max == 0) ? 1 : max;
    rgb_init(out, r*255/max, g*255/max, b*255/max);
}

inline int rgb_eq(struct rgb *a, struct rgb *b) {
    return memcmp(a, b, sizeof(struct rgb)) == 0;
}

inline int rgb_nz(struct rgb *a) {
    struct rgb b;
    rgb_init(&b, 0, 0, 0);
    return !rgb_eq(a, &b); 
}

int get_base_height(int col) {
    if (col < 16) {
        return ((col % 8) < 4) ? 3 : 0;
    } else {
        return ((col % 8) < 4) ? 0 : 3;
    }
}
