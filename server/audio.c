#include <ao/ao.h>
#include <sndfile.h>
#include <string.h>

int play() {
    ao_device *device;
    ao_sample_format format;
    SF_INFO sfinfo;

    int default_driver;

    short *buffer;
    uint_32 buf_size;


    SNDFILE *file = sf_open("test.wav", SFM_READ, &sfinfo);


    printf("Samples: %d\n", sfinfo.frames);
    printf("Sample rate: %d\n", sfinfo.samplerate);
    printf("Channels: %d\n", sfinfo.channels);

    ao_initialize();

    default_driver = ao_default_driver_id();


    memset(&format, 0, sizeof(format));

    switch (sfinfo.format & SF_FORMAT_SUBMASK) {
        case SF_FORMAT_PCM_16:
            format.bits = 16;
            break;
        case SF_FORMAT_PCM_24:
            format.bits = 24;
            break;
        case SF_FORMAT_PCM_32:
            format.bits = 32;
            break;
        case SF_FORMAT_PCM_S8:
            format.bits = 8;
            break;
        case SF_FORMAT_PCM_U8:
            format.bits = 8;
            break;
        default:
            format.bits = 16;
            break;
    }

    format.channels = sfinfo.channels;
    format.rate = sfinfo.samplerate;
    format.byte_format = AO_FMT_NATIVE;

    device = ao_open_live(default_driver, &format, NULL);

    if (device == NULL) {
        fprintf(stderr, "Error opening device. %d \n", errno);
        return 1;
    }


    buf_size = (uint_32) (format.channels * sfinfo.frames * sizeof(short));
    buffer = calloc(buf_size, sizeof(char));

    printf("buffer-size: %d\n", buf_size);

    printf("Loading file into buffer...\n");
    sf_readf_short(file, buffer, buf_size);
    printf("Closing file...\n");
    sf_close(file);
    printf("Playback...\n");
    ao_play(device, buffer, buf_size);
    printf("Playback finished...\n");

//    long BUFFER_SIZE = (sfinfo.samplerate * 2) * sizeof(short);
//
//    while(1) {
//        int read = sf_read_short(file, buffer, BUFFER_SIZE / 4);
//
//        if(read <= 0) {
//            printf("** File has ended (readcount: %ld/%ld)!\n", read, (BUFFER_SIZE / 4));
//            break;
//        }
//
//        ao_play(device, (char *)buffer, (BUFFER_SIZE / 2));
//    }


    ao_close(device);
    ao_shutdown();


    return (0);
}
