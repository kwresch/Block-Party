struct WAV_header {
    char    riff[4];
    int     size;
    char    wave[4];

    char    format_marker[4];
    short   format_marker_len;
    short   format;
    short   num_channels;
    int     sample_rate;
    int     byte_rate;
    short   block_align;
    short   bits_per_sample;

    char    data_marker[4];
    int     data_size;
};