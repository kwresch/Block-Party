#include <alsa/asoundlib.h>
#include "audio_capture.h"

int get_audio_data(char *device, int channels, char *data, int datasize) {
    int i;
    int err;
    char buf[128];
    int rate = 44100;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    
    if ((err = snd_pcm_open(&capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "Cannot open audio device %s (%s)\n", device, snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        fprintf(stderr, "Cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
        fprintf(stderr, "Cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "Cannot set access type (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf(stderr, "Cannot set sample format (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf(stderr, "Cannot set sample rate (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, channels)) < 0) {
        fprintf(stderr, "Cannot set channel count (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
        fprintf(stderr, "Cannot set parameters (%s)\n", snd_strerror(err));
        exit(1);
    }

    snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare(capture_handle)) < 0) {
        fprintf(stderr, "Cannot prepare audio interface for use (%s)\n", snd_strerror(err));
        exit(1);
    }

    i = 0;
    fprintf(stdout, "DATASIZE: %d, DEV: %s, CHANNELS: %d\n", datasize, device, channels);
    int frames = 128 / 4;

    while ((i + 128) < datasize) {
        if ((err = snd_pcm_readi(capture_handle, buf, frames)) != frames) {
            fprintf(stderr, "Read from audio interface failed (%s)\n", snd_strerror(err));
            exit(1);
        }
        memcpy((data + i), buf, 128);
        i += 128;
        memset(data, 0, 128);
        fprintf(stdout, "LOOP: %d\n", (i * 100 / 1764000));
    }
    
    fprintf(stdout, "OUT i=%d\n", i);
    snd_pcm_close(capture_handle);
    fprintf(stdout, "Capture Handle closed\n");
    return 0;
}