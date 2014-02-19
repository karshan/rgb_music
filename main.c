/*
  Copyright (C) 2003-2013 Paul Brossier <piem@aubio.org>

  This file is part of aubio.

  aubio is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  aubio is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with aubio.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "rgb.h"
#include "utils.h"
#define PROG_HAS_PITCH 1
#define PROG_HAS_TEMPO 1
#define PROG_HAS_OUTPUT 1
#define PROG_HAS_JACK 1
#include "parse_args.h"

aubio_pitch_t *pitch;
aubio_tempo_t *tempo;
aubio_wavetable_t *wavetable;
fvec_t *tempo_out;
fvec_t *pitch_out;
smpl_t is_beat = 0;
uint_t is_silence = 0.;
uint_t my_last_beat = 0; 
uint_t total_frames = 0; 
uint_t samples_per_beat = 0; 

void process_block(fvec_t * ibuf, fvec_t *obuf) {
  aubio_tempo_do (tempo, ibuf, tempo_out);
  is_beat = fvec_get_sample (tempo_out, 0);
  //smpl_t bpm = aubio_tempo_get_bpm(tempo); 
  //uint_t last_beat = aubio_tempo_get_last(tempo);

  aubio_pitch_do (pitch, ibuf, pitch_out);
  smpl_t freq = fvec_get_sample(pitch_out, 0);

  if (silence_threshold != -90.)
    is_silence = aubio_silence_detection(ibuf, silence_threshold);
  fvec_zeros (obuf);
  if ( is_beat && !is_silence ) {
    samples_per_beat = total_frames - my_last_beat;
    my_last_beat = total_frames;

    aubio_wavetable_play ( wavetable );
  } else {
    aubio_wavetable_stop ( wavetable );
  }

  if (mix_input)
    aubio_wavetable_do (wavetable, ibuf, obuf);
  else
    aubio_wavetable_do (wavetable, obuf, obuf);

  rgb_iterate(my_last_beat, samples_per_beat, total_frames, freq);

  total_frames += hop_size;
}

void process_print (void) {
  if ( is_beat && !is_silence ) {
    outmsg("%f\n", aubio_tempo_get_last_s(tempo) );
  }
}

int main(int argc, char **argv) {
  // override general settings from utils.c
  buffer_size = 1024;
  hop_size = 512;

  rgb_init();

  examples_common_init(argc,argv);

  verbmsg ("using source: %s at %dHz\n", source_uri, samplerate);

  verbmsg ("tempo method: %s, ", tempo_method);
  verbmsg ("buffer_size: %d, ", buffer_size);
  verbmsg ("hop_size: %d, ", hop_size);
  verbmsg ("threshold: %f\n", onset_threshold);

  tempo_out = new_fvec(2);
  tempo = new_aubio_tempo(tempo_method, buffer_size, hop_size, samplerate);
  if (onset_threshold != 0.) aubio_tempo_set_threshold (tempo, onset_threshold);

  pitch = new_aubio_pitch (pitch_method, buffer_size, hop_size, samplerate);
  if (pitch_tolerance != 0.)
    aubio_pitch_set_tolerance (pitch, pitch_tolerance);
  if (silence_threshold != -90.)
    aubio_pitch_set_silence (pitch, silence_threshold);
  if (pitch_unit != NULL)
    aubio_pitch_set_unit (pitch, pitch_unit);

  pitch_out = new_fvec (1);



  wavetable = new_aubio_wavetable (samplerate, hop_size);
  aubio_wavetable_set_freq ( wavetable, 2450.);
  //aubio_sampler_load (sampler, "/archives/sounds/woodblock.aiff");

  examples_common_process((aubio_process_func_t)process_block,process_print);

  del_aubio_tempo(tempo);
  del_aubio_wavetable (wavetable);
  del_fvec(tempo_out);

  del_aubio_pitch (pitch);
  del_fvec (pitch_out);

  examples_common_del();
  return 0;
}

