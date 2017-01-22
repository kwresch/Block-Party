#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

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

// int main() {
//     int channels = 2;
//     struct WAV_header *wh = (struct WAV_header *) malloc(sizeof(struct WAV_header));
//     build_header(wh, channels);
//     printf("DATA: %s\n", WAV_header_data_block);
//     int datasize = WAV_SAMPLES * channels * BITS_PER_SAMPLE / 8;
//     char data[datasize];
//     printf("datasize: %d\n", datasize);
//     char device[] = "loop\0";
//     int ret = get_audio_data(device, channels, data, datasize);
//     printf("get_audio_data returned\n");
//     fflush(stdout);
//     wh->size = wh->data_size + 36;
//     write_WAV("/home/kdw/BoilerMake/testfiles/test.wav", wh, data, datasize);
// }

void start_stream(int output, int channels) {
    fprintf(stderr, "start_stream() called\n");
    int pipes[2], pid;
    pipe(pipes);

        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Unable to fork\n");
            exit(1);
        }
        if (pid == 0) {
            // Child Process
            close(pipes[0]);
            dup2(pipes[1], 1);
            close(pipes[1]);

            char *args[5];
            args[0] = "pacat";
            args[1] = "--record";
            args[2] = "-d";
            args[3] = MONITOR;
            args[4] = NULL;
            fprintf(stderr, "Starting pacat\n");

            int ret = execvp(args[0], args);
            if (ret < 0) {
                fprintf(stderr, "Error pacat %d -- %s\n", errno, strerror(errno));
            }
        } else {
            // Parent Process
            close(pipes[1]);
            dup2(pipes[0], 0);
            close(pipes[0]);
            dup2(output, 1);
            close(output);

            char *args[25];
            args[0] = "sox";
            args[1] = "-t";
            args[2] = "raw";
            args[3] = "-r";
            args[4] = (char *) malloc(sizeof(char) * SAMPLE_RATE / 10 + 1);
            sprintf(args[4], "%d", SAMPLE_RATE);
            args[5] = "-e";
            args[6] = "signed-integer";
            args[7] = "-L";
            args[8] = "-b";
            args[9] = (char *) malloc(sizeof(char) * BITS_PER_SAMPLE / 10 + 1);
            sprintf(args[9], "%d", BITS_PER_SAMPLE);
            args[10] = "-c";
            args[11] = (char *) malloc(sizeof(char) * 2);
            sprintf(args[11], "%d", channels);
            args[12] = "-";
            args[13] = "-t";
            args[14] = "raw";
            args[15] = "-r";
            args[16] = (char *) malloc(sizeof(char) * SAMPLE_RATE / 10 + 1);
            sprintf(args[16], "%d", SAMPLE_RATE);
            args[17] = "-e";
            args[18] = "signed-integer";
            args[19] = "-b";
            args[20] = "16";
            args[21] = "-c";
            args[22] = (char *) malloc(sizeof(char) * 2);
            sprintf(args[22], "%d", channels);
            args[23] = "-";
            args[24] = NULL;
            fprintf(stderr, "Starting sox\n");

            // usleep(20000);

            int ret = execvp(args[0], args);
            if (ret < 0) {
                fprintf(stderr, "Error sox %d -- %s\n", errno, strerror(errno));
            }
        }
}

void stream_audio(int sock, int input, int channels) {
    char buff[64];

    // Construct the WAV header
    struct WAV_header *wh = (struct WAV_header *) malloc(sizeof(struct WAV_header));
    build_header(wh, channels);
    int datasize = WAV_SAMPLES * channels * BITS_PER_SAMPLE / 8;
    char data[datasize];
    wh->size = wh->data_size + 36;

    // HTTP header
    char http_head[144];
    sprintf(http_head, "HTTP/1.1 200 OK\nServer: WreschServer\nContent-Type: audio/wav\nContent-Length: %d\nAccept-Ranges: bytes\nConnection: close\n\n", wh->size);
    // char *http_head =
    // "HTTP/1.1 200 OK\n"
    // "Server: WreschServer\n"
    // "Content-Type: audio/wav\n"
    // "Content-Length: \n"
    // "Accept-Ranges: bytes\n"
    // "Connection: close\n"
    // "\n";

    // Clear Pipe Buffer
    char pipe_buf[128];
    int j = 0;
    while(j > 64) {
        j = read(input, pipe_buf, 128);
    }
    fprintf(stderr, "Buffer Cleared\n");

    // Write http header to socket
    int n = write(sock, http_head, strlen(http_head));
    // Write header to socket
    n = write(sock, wh, sizeof(struct WAV_header));
    int i = 0;
    // fflush(input);
    while(i < datasize) {
        n = read(input, buff, 64);
        // printf("READ RESPONSE: %d\n", n);
        n = write(sock, buff, 64);
        // printf("WRITE RESPONSE: %d\n", n);
        memset(buff, 0, 64);
        i += 64;
    }
    printf("Wrote %d bytes\n", i);
}