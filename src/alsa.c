#include "settings.h"

/** TODO
 * 1. Convert audio_data in proper buffers
 * 2. Treat stream as mono
 * 3. Modify main_event to support polling from this audio interface API
*/

static void initialize_audio_parameters(snd_pcm_t** handle, struct audio_data* audio,
snd_pcm_uframes_t* frames) {

	// open alsa device to capture
	int err = snd_pcm_open(handle, audio->source, SND_PCM_STREAM_CAPTURE, 0);
	if (err < 0) {
		fprintf(stderr, "error opening stream: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	snd_pcm_hw_params_t* params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(*handle, params);

	// interleaved
	snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	// set 16bit
	snd_pcm_hw_params_set_format(*handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(*handle, params, ALSA_CHANNELS);

	// set sample rate
	unsigned int sample_rate = ALSA_SAMPLE_RATE;
	snd_pcm_hw_params_set_rate_near(*handle, params, &sample_rate, NULL);

	// number of frames pr read
	snd_pcm_hw_params_set_period_size_near(*handle, params, frames, NULL);

	if((err = snd_pcm_hw_params(*handle, params)) < 0); // attempting to set params
		fprintf(stderr, "[ALSA]: Cannot set parameters: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

    if ((err = snd_pcm_prepare (*handle)) < 0) {
    	fprintf (stderr, "[ALSA] Cannot prepare audio interface: (%s)\n", snd_strerror (err));
    	exit (EXIT_FAILURE);
    }

	// getting actual format
	snd_pcm_hw_params_get_format(params, (snd_pcm_format_t*)&sample_rate);

	// converting result to number of bits
	if (sample_rate <= 5) 		audio->format = 16;
	else if (sample_rate <= 9 )	audio->format = 24;
	else 						audio->format = 32;

	snd_pcm_hw_params_get_rate(params, &audio->rate, NULL);
	snd_pcm_hw_params_get_period_size(params, frames, NULL);
}

static int get_certain_frame(signed char* buffer, int buffer_index, int adjustment) {
	// using the 10 upper bits this would give me a vert res of 1024, enough...
	int temp = buffer[buffer_index + adjustment - 1] << 2;
	int lo = buffer[buffer_index + adjustment - 2] >> 6;
	if (lo < 0)
		lo = abs(lo) + 1;
	if (temp >= 0)
		temp += lo;
	else
		temp -= lo;
	return temp;
}

static void fill_audio_outs(struct audio_data* audio, signed char* buffer,
const int size) {
	int radj = audio->format / 4; // adjustments for interleaved
	int ladj = audio->format / 8;
	static int audio_out_buffer_index = 0;
	// sorting out one channel and only biggest octet
	for (int buffer_index = 0; buffer_index < size; buffer_index += ladj * 2) {
		// first channel
		int tempr = get_certain_frame(buffer, buffer_index, radj);
		// second channel
		int templ = get_certain_frame(buffer, buffer_index, ladj);

		// mono: adding channels and storing it in the buffer
		if (audio->channels == 1)
			audio->audio_out_l[audio_out_buffer_index] = (templ + tempr) / 2;
		else { // stereo storing channels in buffer
			audio->audio_out_l[audio_out_buffer_index] = templ;
			audio->audio_out_r[audio_out_buffer_index] = tempr;
		}

		++audio_out_buffer_index;
		audio_out_buffer_index %= 2048;
	}
}

#define FRAMES_NUMBER 256


void* input_alsa(void* buffer) {
	int err;
	snd_pcm_sframes_t* audio = (snd_pcm_sframes_t*)buffer;
	snd_pcm_t* handle;
	snd_pcm_uframes_t buffer_size;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t frames = FRAMES_NUMBER;

	initialize_audio_parameters(&handle, audio, &frames);
	snd_pcm_get_params(handle, &buffer_size, &period_size);

	int16_t buf[period_size];
	frames = period_size / ((audio->format / 8) * ALSA_CHANNELS);
	//printf("period size: %lu\n", period_size);
	//exit(0);

	// frames * bits/8 * channels
	//const int size = frames * (audio->format / 8) * ALSA_CHANNELS;
	signed char* buffer = malloc(period_size);
	int n = 0;

	while (1) {
        switch (audio->format) {
        case 16:
            err = snd_pcm_readi(handle, buf, frames);
	    for (int i = 0; i < frames * 2; i += 2) {
		    buffer[n] = (buf[i] + buf[i + 1]) / 2;
            break;
        default:
		err = snd_pcm_readi(handle, buffer, frames);
		fill_audio_outs(audio, buffer, period_size);
            break;
        }

	    if (err == -EPIPE) {
		    /* EPIPE means overrun */
		    #ifdef DEBUG
			    fprintf(stderr, "overrun occurred\n");
		    #endif
        snd_pcm_prepare(handle);
	    } else if (err < 0) {
		    #ifdef DEBUG
			    fprintf(stderr, "error from read: %s\n", snd_strerror(err));
		    #endif
	    } else if (err != (int)frames) {
		    #ifdef DEBUG
			    fprintf(stderr, "short read, read %d %d frames\n", err, (int)frames);
		    #endif
	    }




		if (audio->terminate == 1) {
			free(buffer);
			snd_pcm_close(handle);
			return NULL;
		}
	}
}
