#ifndef RGB_H
#define RGB_H

extern void rgb_init(void);
extern void rgb_iterate(unsigned int last_beat, unsigned int samples_per_beat, unsigned int sample_no, float freq);

#endif
