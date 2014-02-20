#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include "rgb.h"

struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

long long int tv_to_ms(struct timeval *a) {
    return (a->tv_sec * 1000 + a->tv_usec/1000);
}

long long int tv_diff(struct timeval *a, struct timeval *b) {
    return tv_to_ms(a) - tv_to_ms(b);
}

#define BEATS_LEN 64
#define MIN_BPM 60
struct timeval beats[BEATS_LEN];
int beat_start = 0;
int beat_end = 0;
void beat_add() {
    gettimeofday(&beats[beat_end], 0); 
    beat_end = (beat_end + 1) % BEATS_LEN;
    if (beat_end == beat_start) {
        beat_start = (beat_start + 1) % BEATS_LEN;
    }
}

double compute_ms_beat() {
    struct timeval tv_a;
    int start = (beat_start + 1) % BEATS_LEN;
    long long int diffs[BEATS_LEN];
    int i, n, diffs_i = 0;
    double out;

    tv_a = beats[beat_start];
    while (start != beat_end) {
        diffs[diffs_i++] = tv_diff(&beats[start], &tv_a);
        tv_a = beats[start];
        start = (start + 1) % BEATS_LEN;
    }

    n = 0;
    for (i = 0; i < diffs_i; i++) {
        if (diffs[i] > (60/MIN_BPM)*1000) {
            out = 0;
            n = 0;
        } else {
            out += diffs[i];
            n++;
        }
    }
    if (n == 0) {
        return 0.;
    } else {
        return out/n;
    }
}

#define DISPLAY_RLIMIT 128
int energy = 10;
double ms_beat = 0;
int the_multiplier = 1;
int simple_iterations = 0;
void display_data() {
    int i;
    printf("\033c");
    printf("enrgy:%d\tms/b:%lf\tmult:%d\n\n", energy, ms_beat, the_multiplier);

    fflush(0);
}

void clip_data() {
    if (the_multiplier <= 0) the_multiplier = 1;

    if (energy <= 0) {
        energy = 1;
    } else if (energy > 100) {
        energy = 100;
    }
}

int main() {
    int i, j;
    char c;
    double tmp_dbl;
    struct timeval now, last_beat = {0};

    set_conio_terminal_mode();
    rgb_music_init();

    while(1) {
        tmp_dbl = compute_ms_beat();
        if (tmp_dbl > 0.01) ms_beat = tmp_dbl;

        gettimeofday(&now, 0);

        if (tv_diff(&now, &last_beat) > ms_beat/the_multiplier) {
            rgb_music_iterate(5, 1, 5, energy);
            last_beat = now;
        }
        
        if (kbhit()) { 
            c = getch();
           
            if (c == 'q') {
                energy += 5;
            } else if (c == 'w') {
                energy -= 5;
            } else if (c == 'e') {
                the_multiplier <<= 1;
            } else if (c == 'r') {
                the_multiplier >>= 1;
            } else if (c == 'b') {
                beat_add();
                if (tv_diff(&now, &last_beat) < ms_beat/the_multiplier/2) {
                    last_beat = now;
                }
            } 
            
            else if (c == 'x') {
                break;
            }
        }

        clip_data();

        if (simple_iterations % DISPLAY_RLIMIT == 0) {
            display_data();
        }

        simple_iterations++;
    }

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            rgb_init(&table[i][j], 0, 0, 0);
        }
    }
    draw_table();

    reset_terminal_mode();
    return 0;
}
