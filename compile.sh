#gcc -g jackio.c utils.c rgb.c rgb_util.c visuals.c -I/usr/local/include/aubio /usr/local/lib/libaubio.so -ljack main.c -o main
gcc -g rgb.c rgb_util.c visuals.c simple.c -o simple
