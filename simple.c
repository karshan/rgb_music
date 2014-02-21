#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <string.h>
#include "rgb.h"

extern struct song songs[];
extern int songs_len;

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

#define DISPLAY_RLIMIT 4096*4
double ms_beat = 0;
int simple_iterations = 0;
int song_no = 0;
int effect_no = 0;
extern int usb_fd;
void display_data() {
    int i, j;
    printf("\033c");
    printf("enrgy:%d\r\n", songs[song_no].vps[effect_no].energy);
    printf("song:%d\r\n", song_no);
    printf("effect:%d\r\n", effect_no);
    printf("dir:%d\r\n", songs[song_no].vps[effect_no].dir);
    printf("mult:%d\r\n", songs[song_no].vps[effect_no].multiplier);
    printf("\r\nbpm:%lf\r\n", 1./(((ms_beat)/1000.0)/60.0));
    printf("\r\nusb_fd:%d\r\n", usb_fd);
    for (i = ROWS_E - 1; i >= 0; i--) {
        for (j = 0; j < COLS; j++) {
            printf("\x1b[0;");
            if (table[i][j].r > 0 && table[i][j].g > 0 && table[i][j].b > 0) {
                printf("37;");
            } else if (table[i][j].r > 0 && table[i][j].g > 0) {
                printf("33;");
            } else if (table[i][j].r > 0 && table[i][j].b > 0) {
                printf("35;");
            } else if (table[i][j].g > 0 && table[i][j].b > 0) {
                printf("36;");
            } else if (table[i][j].r > 0) {
                printf("31;");
            } else if (table[i][j].g > 0) {
                printf("32;");
            } else if (table[i][j].b > 0) {
                printf("34;");
            }
            printf("40m#");
        }
        printf("\r\n");
    }
    printf("\x1b[0;30;47m");

    fflush(0);
}

void clip_data() {
    if (songs[song_no].vps[effect_no].multiplier <= 0) {
        songs[song_no].vps[effect_no].multiplier = 1;
    } else if(songs[song_no].vps[effect_no].multiplier > 64) {
        songs[song_no].vps[effect_no].multiplier = 64;
    }

    if (songs[song_no].vps[effect_no].energy < 0) {
        songs[song_no].vps[effect_no].energy = 0;
    } else if (songs[song_no].vps[effect_no].energy > 100) {
        songs[song_no].vps[effect_no].energy = 100;
    }

    if (song_no < 0) {
        song_no = 0;
    } else if (song_no >= songs_len) {
        song_no = songs_len - 1;
    }

    if (effect_no < 0) {
        effect_no = 0;
    } else if (effect_no >= songs[song_no].effects_len) {
        effect_no = songs[song_no].effects_len - 1;
    }
}

int actual_iterations = 0;
char trans_on = 0;
int trans_start = 0;

extern void draw_table();
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
        ms_beat = 1.0/((128.1/60)/1000.0);

        gettimeofday(&now, 0);

        if (ms_beat != 0. && (tv_diff(&now, &last_beat) > ms_beat/songs[song_no].vps[effect_no].multiplier)) {
            fill_table(song_no, effect_no);
            draw_table();
            last_beat = now;
            actual_iterations++;

#if 0
            if (trans_on == 1) {
                if ((actual_iterations - trans_start) >= 2) {
                    cgen = 0;
                    the_multiplier = 32;
                    effect = 9;
                    trans_on = 2;
                    trans_start = actual_iterations;
                }
            } else if (trans_on == 2) {
                if (actual_iterations - trans_start == COLS) {
                    dir = !dir;
                    effect = 10;
                } else if (actual_iterations - trans_start >= COLS*2) {
                    effect = 6;
                    the_multiplier = 8;
                    energy = 75;
                    cgen = 10;
                    trans_on = 0;
                }
            }
#endif
        }
        
        if (kbhit()) { 
            c = getch();
           
            if (c == 'q') {
                songs[song_no].vps[effect_no].energy += 5;
            } else if (c == 'w') {
                songs[song_no].vps[effect_no].energy -= 5;
            } else if (c == 'a') {
                songs[song_no].vps[effect_no].energy += 50;
            } else if (c == 's') {
                songs[song_no].vps[effect_no].energy -= 50;
            } else if (c == 'e') {
                songs[song_no].vps[effect_no].multiplier <<= 1;
            } else if (c == 'r') {
                songs[song_no].vps[effect_no].multiplier >>= 1;
            } else if (c == 't') {
                effect_no++;
            } else if (c == 'y') {
                effect_no--;
            } else if (c == 'u') {
                song_no++;
                effect_no = 0;
            } else if (c == 'i') {
                song_no--;
                effect_no = 0;
            } else if (c == 'p') {
                songs[song_no].vps[effect_no].dir = !songs[song_no].vps[effect_no].dir;
            } else if (c == 'g') {
                /*trans_start = actual_iterations;
                trans_on = 1;
                the_multiplier = 1;*/
            } else if (c == 'b') {
                beat_add();
                if (tv_diff(&now, &last_beat) < ms_beat/songs[song_no].vps[effect_no].multiplier/2) {
                    last_beat = now;
                }
            } 
            
            else if (c == 'X') {
                break;
            }
        }

        clip_data();

        if ((simple_iterations % DISPLAY_RLIMIT) == 0) {
            display_data();
        }

        simple_iterations++;
    }

    for (i = 0; i < ROWS_E; i++) {
        for (j = 0; j < COLS; j++) {
            rgb_init(&table[i][j], 0, 0, 0);
        }
    }
    draw_table();

    reset_terminal_mode();
    return 0;
}
