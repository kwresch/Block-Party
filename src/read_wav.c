#include <stdio.h>

main(void) {
    FILE* fp;

    fp = fopen("/home/kdw/BoilerMake/testfiles/test.wav", "r");

    int c;
    int i = 0;
    while ((c = fgetc(fp)) != EOF && (i < 48)) {
        printf("%x ", c);
        i++;
    }
}