/** @file simple_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 */

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>
#include <fftw3.h>

#define MAG_SCALE       10000

#define SAMPLE_SIZE     1024
#define SAMPLE_RATE     48000
#define FFT_SIZE        (SAMPLE_SIZE / 2)
#define MAX_FREQ        8000
#define HIST_SIZE       13
#define FREQ_PER_BIN    (SAMPLE_RATE / SAMPLE_SIZE)
#define FFT_NUM_BINS    (MAX_FREQ/FREQ_PER_BIN)
#define FFT_BIN_WIDTH   10              // pixel width of bin

#define COLS 12
#define ROWS 4
#define COLS_P 4
#define ROWS_P 4
#define SQUARES 3
#define BUFFER_SIZE (2*(ROWS*COLS*3 + SQUARES))

char initted = 0;

struct rgb {
    char r;
    char g;
    char b;
};

struct rgb table[ROWS][COLS*SQUARES];
unsigned char square_ids[SQUARES] = { 0x5, 0x0, 0x2a };

int i,j,k;

double MAG_TRIGGER = 0.36; 
double VAR_TRIGGER = 0.36; 
double clip_mag;
double clip_mag_decay;
char clipped;        

struct bin
{
    double mag;
    double last_mag;
    double diff;
    
    double hist[HIST_SIZE];
    double hist_avg;
    double hist_std;

    char triggered;
    char trigger_hist[HIST_SIZE];

    char is_pulse;
};

struct bin fft_bin[FFT_NUM_BINS];

double fft_global_mag_avg;
double fft_global_mag_max;
double fft_global_hist_mag_avg;  // average of all the bin history averages
double fft_global_hist_mag_max;  // max value of global history
double fft_global_hist_std_avg;  // avg of all the std deviations
double fft_global_hist_std_max;  // max of all the std deviations

jack_port_t *input_port;
jack_client_t *client;

double *fft_input;
fftw_complex *fft_out;
fftw_plan fft_plan;

int usb_fd;

int init_fft(char *fname)
{
    fft_input = fftw_malloc(sizeof(double) * SAMPLE_SIZE);
    fft_out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    fft_plan = fftw_plan_dft_r2c_1d(SAMPLE_SIZE, fft_input, fft_out, FFTW_ESTIMATE);

    usb_fd = open(fname, O_WRONLY);
    printf("usb_fd = %d\n", usb_fd);

    return 0;
}

char buffer[BUFFER_SIZE];
void draw_table(void) {
    int p = 0;
    for (i = 0; i < SQUARES; i++) {
        buffer[p++] = 0xf0;
        buffer[p++] = square_ids[i];
        for (j = 0; j < ROWS_P; j++) {
            for (k = 0; k < COLS_P; k++) {
                buffer[p++] = 0x1;
                buffer[p++] = table[j][k + COLS_P*i].r;
                buffer[p++] = 0x1;
                buffer[p++] = table[j][k + COLS_P*i].b;
                buffer[p++] = 0x1;
                buffer[p++] = table[j][k + COLS_P*i].g;
            }
        }
    }
    write(usb_fd, buffer, BUFFER_SIZE);
}

void copy_bins_to_old(void)
{
    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        fft_bin[i].last_mag = fft_bin[i].mag;
    }
}

// takes the magnitude of the fft output
// scale the magnitude down
// clip it if desired
void compute_magnitude(void)
{
    // we have not clipped a signal until proven otherwise
    clipped = 0;

    static double clip_mag_raw = 0;

    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        // compute magnitude magnitude
        fft_bin[i].mag = sqrt( fft_out[i][0]*fft_out[i][0] + fft_out[i][1]*fft_out[i][1] );

        // scale it
        fft_bin[i].mag /= (MAG_SCALE);

        // find new clip
        if (fft_bin[i].mag > clip_mag_raw)
        {
            clip_mag_raw = fft_bin[i].mag;    
        }
    }

    // check if we want to use a dynamic clip or not
    clip_mag = (clip_mag_raw * 1 / 2);

    // loop through all bins and see if they need to be clipped
    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        if (fft_bin[i].mag > clip_mag)
        {
            fft_bin[i].mag = clip_mag;
            clipped = 1;
        }    
    }

    // if we haven't clipped a bin this time
    // we want to start lowering the clip magnitude
    if (! clipped) 
    {
        clip_mag_raw -= pow(clip_mag_decay/100, 3);    // decrease the clip by x^3 since it has slow 'acceleration'
        clip_mag_decay++;                            // increment the index 
    }
    else
        clip_mag_decay = 0;                            // we did clip a bin so reset the decay

    // calculate the average magnitude
    fft_global_mag_avg = 0;
    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        fft_global_mag_avg += fft_bin[i].mag;
    }
    fft_global_mag_avg /= FFT_NUM_BINS;

    // if a new song comes on we want to reset the clip
    // we can tell a new song by seeing if the avg magnitude is close to zero
    if (fft_global_mag_avg < 0.005)
    {
        clip_mag_raw = 0;
        clip_mag_decay = 0;
    }
}

// take the difference between these bins are the last
// TODO: use fft_bin_hist[i][HIST_SIZE-2]
//     would only work if noting use delta for everything
void compute_delta_from_last(void)
{
    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        // calculate difference between these bins and the last
        // only takes the ones that increased from last time
        if (fft_bin[i].mag < fft_bin[i].last_mag)
            fft_bin[i].diff = 0;
        else
            fft_bin[i].diff = fft_bin[i].mag - fft_bin[i].last_mag;
    }
}

// push the current bins into history
void add_bins_to_history (void)
{
    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        // shift history buffer down
        for (k = 1; k<HIST_SIZE; k++)
        {
            fft_bin[i].hist[k-1] = fft_bin[i].hist[k];
        }

        // add current value to the history
        fft_bin[i].hist[HIST_SIZE-1] = fft_bin[i].mag;
    }
}

// calculate average for each bin history
// calculate average of all bin history averages
// find the global maximum magnitude 
void compute_bin_hist(void)
{
    fft_global_hist_mag_max = 0;
    fft_global_hist_mag_avg = 0;

    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        // reset bin hist avg
        fft_bin[i].hist_avg = 0;

        for (k = 1; k<HIST_SIZE; k++)
        {
            // sum all the values in this bin hist
            fft_bin[i].hist_avg += fft_bin[i].hist[k];

            // sum all values for global average
            fft_global_hist_mag_avg += fft_bin[i].hist[k];

            // check to see if we found new global max
            if (fft_bin[i].hist[k] > fft_global_hist_mag_max)  fft_global_hist_mag_max = fft_bin[i].hist[k];
        }

        // average this bin
        fft_bin[i].hist_avg /= HIST_SIZE;
    }

    // average global
    fft_global_hist_mag_avg /= (FFT_NUM_BINS * HIST_SIZE);
}

// takes the standard deviation of each bins history
// calculates the global average of the std devs
// calculates the global max std dev
void compute_std_dev(void)
{
    fft_global_hist_std_max = 0;
    fft_global_hist_std_avg = 0;

    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        // compute the std deviation
        // sqrt(1/n * sum(x - a)^2)

        fft_bin[i].hist_std = 0;

        for (k = 0; k < HIST_SIZE; k++)
        {
            fft_bin[i].hist_std += pow(fft_bin[i].hist[k] - fft_bin[i].hist_avg, 2);
        }

        fft_bin[i].hist_std /= HIST_SIZE;

        fft_bin[i].hist_std = sqrt(fft_bin[i].hist_std);

        // sum each one for average
        fft_global_hist_std_avg += fft_bin[i].hist_std;

        // see if we found a new maximum
        if (fft_bin[i].hist_std > fft_global_hist_std_max) fft_global_hist_std_max = fft_bin[i].hist_std;
    }

    // take average
    fft_global_hist_std_avg /= FFT_NUM_BINS;
}

void detect_beats(void)
{
    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        // shift trigger history down
        for (k=1; k < HIST_SIZE; k++)
        {
            fft_bin[i].trigger_hist[k-1] = fft_bin[i].trigger_hist[k];
        }

        // see if we detect a beat
        if (fft_bin[i].mag/fft_global_mag_max > MAG_TRIGGER && fft_bin[i].hist_std/fft_global_hist_std_max > VAR_TRIGGER)
            fft_bin[i].triggered = 1;
        else
            fft_bin[i].triggered = 0;

        // if this bin is decreasing from last time it is no longer a beat
        //if (fft_bin_diff[i] <= 0)
        //    fft_bin_triggered[i] = 0;

        // add current trigger state to hist buffer
        fft_bin[i].trigger_hist[HIST_SIZE-1] = fft_bin[i].triggered;
    }
}

void assign_lights(void)
{
    int pulse_count = 0;
    int center_of_pulse = 0;

    // finds how many groups of pulses there are
    // marks the center of them
    for (i=1; i<FFT_NUM_BINS; i++)
    {
        fft_bin[i].is_pulse = 0;

        /*
        // if this one is triggered and the previous one isn't we found start of group
        if (fft_bin[i].triggered && !fft_bin[i-1].triggered)
        {
            pulse_count++;
            center_of_pulse = i;

        }
        // if it is not triggered but the last one is we found end of group
        else if (!fft_bin[i].triggered && fft_bin[i-1].triggered)
        {
            center_of_pulse = (i-center_of_pulse) / 2  + center_of_pulse;
            fft_bin[center_of_pulse].is_pulse = 1;
        }*/

        fft_bin[i].is_pulse = fft_bin[i].triggered;
    }

    for (i = 0; i < COLS; i++) {
        for (j = 0; j < ROWS; j++) {
            table[j][i].r = 0;
            table[j][i].g = 255;
            table[j][i].b = 0;
            if (fft_bin[i].is_pulse) {
                table[j][i].r = 255;
            }
        }
    }
}

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by 
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int
process (jack_nframes_t nframes, void *arg)
{
        jack_default_audio_sample_t *in;
	in = jack_port_get_buffer (input_port, nframes);

        if (!initted) {
            initted = 1;
            init_fft("/dev/hidraw1");
        }

	for (i = 0; i < SAMPLE_SIZE; i++) {
            fft_input[i] = (double)in[i];
        }

        fftw_execute(fft_plan);

        copy_bins_to_old();

        compute_magnitude();

        compute_delta_from_last();

        add_bins_to_history();

        compute_bin_hist();    

        compute_std_dev();

        detect_beats();
        
        assign_lights();
        
        draw_table();

        for (i=1; i<FFT_NUM_BINS; i++) {
            if (fft_bin[i].is_pulse) {
                printf("#");
            } else {
                printf(".");
            }
        }
        printf("\n");

	return 0;      
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown (void *arg)
{
	exit (1);
}

int
main (int argc, char *argv[])
{
	const char **ports;
	const char *client_name = "simple";
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	/* open a client connection to the JACK server */

	client = jack_client_open (client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (client, process, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate. 
	 */

	printf ("engine sample rate: %" PRIu32 "\n",
		jack_get_sample_rate (client));

	/* create two ports */

	input_port = jack_port_register (client, "input",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);

	if ((input_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}

	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (1);
	}

	if (jack_connect (client, ports[0], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);
	
	/* keep running until stopped by the user */

	sleep (-1);

	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	*/

	jack_client_close (client);
	exit (0);
}
