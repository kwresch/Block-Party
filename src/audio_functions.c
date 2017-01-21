#include <stdio.h>
#include "definitions.h"
#include "file_headers.h"

int build_header(struct WAV_header *wh, int channels) {
    wh->riff = strndup("RIFF", 4);
    // Fill wh->size after file creation
    wh->wave = strndup("WAVE", 4);
    
    wh->format_marker = strndup("fmt\0", 4);
    wh->format_marker_len = 16;
    wh->format = 1;         /* 1 = PCM */
    wh->num_channels = channels;
    wh->sample_rate = SAMPLE_RATE;
    /* byte_rate = sample_rate * bits_per_sample * num_channels / 8 */
    wh->byte_rate = (SAMPLE_RATE * BITS_PER_SAMPLE * channels) / 8;
    /* block_align = bits_per_sample * num_channels / 8 */
    wh->block_align = (BITS_PER_SAMPLE * channels) / 8;
    
    wh->bit_per_sample = BITS_PER_SAMPLE;

    wh->data_marker = strndup("data", 4);
    wh->data_size = WAV_SAMPLES * channels * BITS_PER_SAMPLE / 8;

    return 0;
}

int write_WAV(char *file_path, struct WAV_header *wh, char *data, int datasize) {
    FILE* fp = fopen(file_path, "a+");

    size_t size = fwrite(wh, 1, sizeof(struct WAV_header), fp);
    if (size <= 0) {
        fprintf(stderr, "Cannot write to file\n");
        exit(1);
    }
    size = fwrite(data, 1, datasize, fp);
    if (size <= 0) {
        fprintf(stderr, "Cannot write to file\n");
        exit(1);
    }
}

int main() {
    struct WAV_header *wh = (struct WAV_header *) malloc(sizeof(struct WAV_header));
    build_header(wh, 2);
    int datasize = WAV_SAMPLES * 2 * BITS_PER_SAMPLE / 8;
    char data[datasize];

    write_WAV("/home/kdw/BoilerMake/testfiles/test.wav", wh, data, datasize);
}