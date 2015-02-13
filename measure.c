#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    struct timespec start, stop;
    long int accum = 0;
    long i, j;

    const unsigned int times = 50000;


    for (i = times; i != 0; --i) {
        struct timespec sleep_time = {0, 0};
        nanosleep(&sleep_time, NULL);

//        for (j = 21300; j != 0; --j) {
//            asm("");   // skip gcc
//        }
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);


    for (i = times; i != 0; --i) {
        struct timespec sleep_time = {0, 20000};
        nanosleep(&sleep_time, NULL);


//        for (j = 21300 / 3; j != 0; --j) {
//            asm("");   // skip gcc
//        }
    }

//            struct timespec sleep_time = {0, 0}; -> 21300ns; iter asm: nanos / 3 -> 21300ns


    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);

    printf("Start: %lu\n", (start.tv_nsec + start.tv_sec * 1000000000));
    printf("Stop: %lu\n", (stop.tv_nsec + stop.tv_sec * 1000000000));

    accum = ((stop.tv_nsec + stop.tv_sec * 1000000000) - (start.tv_nsec + start.tv_sec * 1000000000));

    printf("ns: %lu\n", accum / times);

    return 0;
}
