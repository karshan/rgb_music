#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

char mybuf[1024];
int myp = 0;
int myp2 = 0;
void mywrite(int fd, char*buf, int len) {
    memcpy(mybuf+myp, buf, len);
    myp+=len;
}

void myflush(int fd) {
    write(fd, mybuf, myp);
    myp = 0;
}

void sendb(int fd, int tx9, unsigned char add) {
    char buf[2];
    buf[0] = tx9?0xf0:0x01;
    buf[1] = add;
    mywrite(fd, buf, 2);
}

void send_color(unsigned char color) {
    if (myp2 == 0) {
        mybuf[myp++] = 0x01;
        mybuf[myp] = (color & 0xf0) >> 4;
    } else {
        mybuf[myp] |= (color & 0xf0);
        myp++;
    }
    myp2 = !myp2;
}

unsigned char max(unsigned char a, unsigned char b, unsigned char c) {
    unsigned char out = a;
    if (b > out) out = b;
    if (c > out) out = c;
    return out;
}

int main(int argc, char **argv) {
    int i, j, k, state = 0;
    char buf[2];
    int f = -1;
    if (argc < 2) {
        printf("usage: %s address\n", argv[0]);
        exit(-1);
    }
    f = open("/dev/hidraw0", O_WRONLY);
    if (f < 0) {
        f = open("/dev/hidraw1", O_WRONLY);
    }
    if (f < 0) {
        printf("failed to open /dev/hidraw{0,1}\n");
        exit(-1);
    }

    int address;
    sscanf(argv[1], "%x", &address);
    printf("address: %x\n", (char)address);

    struct timeval ta, tb;
    srand(time(0));
    unsigned char ra=0, rb=0, rc=0, mx;

    for (j = 0; j < 12; j++) {
        sendb(f, 1, (char)address);
        for (i = 0; i < 16; i++) {
            ra = rb = rc = 0;
            if (j == 0) {
                ra = 255;
            } else if (j == 1) {
                rb = 255;
            } else if (j == 2) {
                rc = 255;
            } else if (j == 3) {
                if (i >= 0 && i <= 3) {
                    ra = 255;
                    rb = 255;
                    rc = 255;
                }
            } else if (j == 4) {
                if (i >= 4 && i <= 7) {
                    ra = 255;
                    rb = 255;
                    rc = 255;
                }
            } else if (j == 5) {
                if (i >= 8 && i <= 11) {
                    ra = 255;
                    rb = 255;
                    rc = 255;
                }
            } else if (j == 6) {
                if (i >= 12 && i <= 15) {
                    ra = 255;
                    rb = 255;
                    rc = 255;
                }
            } else if (j == 7) {
                if (i % 4 == 0) {
                    ra = 255;
                    rb = 255;
                    rc = 255;
                }
            } else if (j == 8) {
                if (i % 4 == 1) {
                    ra = 255;
                    rb = 255;
                    rc = 255;
                }
            } else if (j == 9) {
                if (i % 4 == 2) {
                    ra = 255;
                    rb = 255;
                    rc = 255;
                }
            } else if (j == 10) {
                if (i % 4 == 3) {
                    ra = 255;
                    rb = 255;
                    rc = 255;
                }
           } 
           else {
                ra = rand()%256; 
                rb = rand()%256; 
                rc = rand()%256; 
                mx = max(ra, rb, rc);
                ra = ra*255/mx;
                rb = rb*255/mx;
                rc = rc*255/mx;
            }
            send_color(ra);
            send_color(rb);
            send_color(rc);
        }
        sendb(f, 1, 0xf0);
        myflush(f);
        getchar();
    }
}

