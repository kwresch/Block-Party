#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "definitions.h"
#include "file_headers.h"
#include "audio_capture.h"

char riff_str[4] = "RIFF";
char wave_str[4] = "WAVE";
char fmt_str[4] = "fmt ";

int build_header(struct WAV_header *wh, int channels) {
    strncpy(wh->riff, riff_str, 4);
    // Fill wh->size after file creation
    strncpy(wh->wave, wave_str, 4);
    
    strncpy(wh->format_marker, fmt_str, 4);
    // wh->format_marker = strndup("fmt\0", 4);
    wh->format_marker_len = 16;
    wh->format = 1;         /* 1 = PCM */
    wh->num_channels = channels;
    wh->sample_rate = SAMPLE_RATE;
    /* byte_rate = sample_rate * bits_per_sample * num_channels / 8 */
    wh->byte_rate = (SAMPLE_RATE * BITS_PER_SAMPLE * channels) / 8;
    /* block_align = bits_per_sample * num_channels / 8 */
    wh->block_align = (BITS_PER_SAMPLE * channels) / 8;
    wh->bits_per_sample = BITS_PER_SAMPLE;

    strncpy(wh->data_marker, "data", 4);
    wh->data_size = WAV_SAMPLES * channels * BITS_PER_SAMPLE / 8;

    return 0;
}

int write_WAV(char *file_path, struct WAV_header *wh, char *data, int datasize) {
    int offset = 0;
    memcpy((WAV_header_data_block + offset), wh->riff, 4);
    offset += 4;
    memcpy((WAV_header_data_block + offset), &wh->size, 4);
    offset += 4;
    memcpy((WAV_header_data_block + offset), wh->wave, 4);
    offset += 4;
    memcpy((WAV_header_data_block + offset), wh->format_marker, 4);
    offset += 4;
    memcpy((WAV_header_data_block + offset), &wh->format_marker_len, 4);
    offset += 4;
    memcpy((WAV_header_data_block + offset), &wh->format, 2);
    offset += 2;
    memcpy((WAV_header_data_block + offset), &wh->num_channels, 2);
    offset += 2;
    memcpy((WAV_header_data_block + offset), &wh->sample_rate, 4);
    offset += 4;
    memcpy((WAV_header_data_block + offset), &wh->byte_rate, 4);
    offset += 4;
    memcpy((WAV_header_data_block + offset), &wh->block_align, 2);
    offset += 2;
    memcpy((WAV_header_data_block + offset), &wh->bits_per_sample, 2);
    offset += 2;
    memcpy((WAV_header_data_block + offset), wh->data_marker, 4);
    offset += 4;

    printf("SIZE: %x\n", *(WAV_header_data_block + 4));
    printf("FORMAT_MARKER_LEN: %x %x %x %x\n", *(WAV_header_data_block + 16), *(WAV_header_data_block + 17), *(WAV_header_data_block + 18), *(WAV_header_data_block + 19));

    FILE* fp = fopen(file_path, "w");

    //size_t size = fwrite(wh, 1, sizeof(struct WAV_header), fp);
    size_t size = fwrite(WAV_header_data_block, 1, 44, fp);
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
    int channels = 2;
    struct WAV_header *wh = (struct WAV_header *) malloc(sizeof(struct WAV_header));
    build_header(wh, channels);
    printf("DATA: %s\n", WAV_header_data_block);
    int datasize = WAV_SAMPLES * channels * BITS_PER_SAMPLE / 8;
    char data[datasize];
    printf("datasize: %d\n", datasize);
    char device[] = "loop\0";
    int ret = get_audio_data(device, channels, data, datasize);
    printf("get_audio_data returned\n");
    fflush(stdout);
    wh->size = wh->data_size + 36;
    write_WAV("/home/kdw/BoilerMake/testfiles/test.wav", wh, data, datasize);
}

void start_stream(int output) {
    int pipes;
    pipe(pipes);

        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Unable to fork\n");
            exit(1);
        }
        if (pid == 0) {
            // Child Process
            close(pipes[0]);
            dup2(1, pipes[1]);

            char *args[5];
            strndup(args[0], "pacat\0", 6);
            strndup(args[1], "--record\0", 9);
            strndup(args[2], "-d\0", 3);
            strndup(args[3], MONITOR, strlen(MONITOR) + 1);
            strndup(args[4], "\0", 1);

            execvp(args[0], args[]);
        } else {
            // Parent Process
            close(pipes[1]);
            dup2(0, pipes[0]);
            dup2(1, output);

            char *args[13];
            strndup(args[0], "sox\0", 4);
            strndup(args[1], "-t\0", 3);
            strndup(args[2], "raw\0", 4);
            strndup(args[3], "-r\0", 3);
            char *sample_rate = itoa(SAMPLE_RATE);
            strdup(args[4], sample_rate);
            strndup(args[5], "-e\0", 3);
            strndup(args[6], "signed-integer\0", 15);
            strndup(args[7], "-L\0", 3);
            strndup(args[8], "-b\0", 3);
            char *bps = itoa(BITS_PER_SAMPLE);
            strdup(args[9], bps);
            strndup(args[10], "-c\0", 3);
            char *chs = itoa(channels);
            strdup(args[11], chs);
            strndup(args[12], "-\0", 2);

            execvp(args[0], args[]);
        }
}

void stream_audio(sock, input, channels) {
    struct WAV_header *wh = (struct WAV_header *) malloc(sizeof(struct WAV_header));
    build_header(wh, channels);
    // printf("DATA: %s\n", WAV_header_data_block);
    int datasize = WAV_SAMPLES * channels * BITS_PER_SAMPLE / 8;
    char data[datasize];
    // printf("datasize: %d\n", datasize);
    // char device[] = "loop\0";
    // int ret = get_audio_data(device, channels, data, datasize);
    // printf("get_audio_data returned\n");
    // fflush(stdout);
    wh->size = wh->data_size + 36;
    int n = write(sock, wh, sizeof(struct WAV_header));
    int i = 0;
    while(i < datasize) {
        
    }
}