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

int energy = 10;
char c;
int main() {
    set_conio_terminal_mode();
    rgb_music_init();

    while(1) {
        rgb_music_iterate(5, 1, 5, energy);
        
        //while(!kbhit());
        if (kbhit()) { 
            c = getch();
           
            if (c == 'q') {
                energy += 10;
            } else if (c == 'w') {
                energy -= 10;
            } else if (c == 'x') {
                break;
            }
        }
        usleep(1600);
    }
    reset_terminal_mode();
    return 0;
}
