#include <ao/ao.h>
#include <sndfile.h>
#include <signal.h>

#define BUFFER_SIZE 8192

int cancel_playback;

void on_cancel_playback(int sig) {
    if (sig != SIGINT) {
        return;
    }

    cancel_playback = 1;
    exit(0);
}

static void clean(ao_device *device, SNDFILE *file) {
    ao_close(device);
    sf_close(file);
    ao_shutdown();
}

int play(char *file_path) {
    ao_device *device;
    ao_sample_format format;
    SF_INFO sfinfo;

    int default_driver;

    short *buffer;

    signal(SIGINT, on_cancel_playback);


    SNDFILE *file = sf_open(file_path, SFM_READ, &sfinfo);

    printf("Samples: %d\n", sfinfo.frames);
    printf("Sample rate: %d\n", sfinfo.samplerate);
    printf("Channels: %d\n", sfinfo.channels);

    ao_initialize();

    default_driver = ao_default_driver_id();

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
    format.matrix = 0;

    device = ao_open_live(default_driver, &format, NULL);

    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
        return 1;
    }

    buffer = calloc(BUFFER_SIZE, sizeof(short));

    while (1) {
        int read = sf_read_short(file, buffer, BUFFER_SIZE);

        if (ao_play(device, (char *) buffer, (uint_32) (read * sizeof(short))) == 0) {
            printf("ao_play: failed.\n");
            clean(device, file);
            break;
        }

        if (cancel_playback) {
            clean(device, file);
            break;
        }
    }

    clean(device, file);

    return 0;
}


//#include <iostream>
//#include <limits>
//#include <stdio.h>
//
//extern "C" {
//#include "libavutil/mathematics.h"
//#include "libavutil/samplefmt.h"
//#include "libavformat/avformat.h"
//#include "libswscale/swscale.h"
//#include <ao/ao.h>
//}
//#define DBG(x) std::cout<<x<<std::endl
//#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
//
//void die(const char *msg) {
//    printf("%s\n", msg);
//    exit(-1);
//}
//
//int main(int argc, char **argv) {
//    const char *input_filename = argv[1];
//
//    av_register_all();
//
//    AVFormatContext *container = avformat_alloc_context();
//    if (avformat_open_input(&container, input_filename, NULL, NULL) < 0) {
//        die("Could not open file");
//    }
//
//    if (avformat_find_stream_info(container, NULL) < 0) {
//        die("Could not find file info");
//    }
//
//    av_dump_format(container, 0, input_filename, false);
//
//    int stream_id = -1;
//    int i;
//    for (i = 0; i < container->nb_streams; i++) {
//        if (container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
//            stream_id = i;
//            break;
//        }
//    }
//    if (stream_id == -1) {
//        die("Could not find Audio Stream");
//    }
//
//    AVDictionary *metadata = container->metadata;
//
//    AVCodecContext *ctx = container->streams[stream_id]->codec;
//    AVCodec *codec = avcodec_find_decoder(ctx->codec_id);
//
//    if (codec == NULL) {
//        die("cannot find codec!");
//    }
//
//    if (avcodec_open2(ctx, codec, NULL) < 0) {
//        die("Codec cannot be found");
//    }
//
//    //initialize AO lib
//    ao_initialize();
//
//    int driver = ao_default_driver_id();
//
//    ao_sample_format sformat;
//    AVSampleFormat sfmt = ctx->sample_fmt;
//    if (sfmt == AV_SAMPLE_FMT_U8 || sfmt == AV_SAMPLE_FMT_U8P) {
//        sformat.bits = 16;
//    } else if (sfmt == AV_SAMPLE_FMT_S16 || sfmt == AV_SAMPLE_FMT_S16P) {
//        sformat.bits = 16;
//    } else if (sfmt == AV_SAMPLE_FMT_S32 || sfmt == AV_SAMPLE_FMT_S32P) {
//        sformat.bits = 16;
//    } else if (sfmt == AV_SAMPLE_FMT_FLT || sfmt == AV_SAMPLE_FMT_FLTP) {
//        sformat.bits = 16;
//    } else if (sfmt == AV_SAMPLE_FMT_DBL || sfmt == AV_SAMPLE_FMT_DBLP) {
//        sformat.bits = 16;
//    } else {
//        DBG("Unsupported format");
//    }
//
//    sformat.channels = ctx->channels;
//    sformat.rate = ctx->sample_rate;
//    sformat.byte_format = AO_FMT_NATIVE;
//    sformat.matrix = 0;
//
//    ao_device *adevice = ao_open_live(driver, &sformat, NULL);
//
//    AVPacket packet;
//    av_init_packet(&packet);
//
//    AVFrame *frame = avcodec_alloc_frame();
//
//    int buffer_size = AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE;
//
//    // MSVC can't do variable size allocations on stack, ohgodwhy
//    uint8_t *buffer = new
//    uint8_t[buffer_size];
//    packet.data = buffer;
//    packet.size = buffer_size;
//
//    uint8_t *samples = new
//    uint8_t[buffer_size];
//    int len;
//    int frameFinished = 0;
//
//    int plane_size;
//
//    while (av_read_frame(container, &packet) >= 0) {
//        if (packet.stream_index == stream_id) {
//            int len = avcodec_decode_audio4(ctx, frame, &frameFinished, &packet);
//            int data_size = av_samples_get_buffer_size(&plane_size, ctx->channels,
//                    frame->nb_samples,
//                    ctx->sample_fmt, 1);
//            uint16_t *out = (uint16_t *) samples;
//
//            if (frameFinished) {
//                int write_p = 0;
//
//                switch (sfmt) {
//
//                    case AV_SAMPLE_FMT_S16P:
//                        for (int nb = 0; nb < plane_size / sizeof(uint16_t); nb++) {
//                            for (int ch = 0; ch < ctx->channels; ch++) {
//                                out[write_p] = ((uint16_t *) frame->extended_data[ch])[nb];
//                                write_p++;
//                            }
//                        }
//                        ao_play(adevice, (char *) samples, (plane_size) * ctx->channels);
//                        break;
//                    case AV_SAMPLE_FMT_FLTP:
//                        for (int nb = 0; nb < plane_size / sizeof(float); nb++) {
//                            for (int ch = 0; ch < ctx->channels; ch++) {
//                                out[write_p] = ((float *) frame->extended_data[ch])[nb] * std::numeric_limits<short>::max();
//                                write_p++;
//                            }
//                        }
//                        ao_play(adevice, (char *) samples, (plane_size / sizeof(float)) * sizeof(uint16_t) * ctx->channels);
//                        break;
//                    case AV_SAMPLE_FMT_S16:
//                        ao_play(adevice, (char *) frame->extended_data[0], frame->linesize[0]);
//                        break;
//                    case AV_SAMPLE_FMT_FLT:
//                        for (int nb = 0; nb < plane_size / sizeof(float); nb++) {
//                            out[nb] = static_cast<short>(((float *) frame->extended_data[0])[nb] * std::numeric_limits<short>::max());
//                        }
//                        ao_play(adevice, (char *) samples, (plane_size / sizeof(float)) * sizeof(uint16_t));
//                        break;
//                    case AV_SAMPLE_FMT_U8P:
//                        for (int nb = 0; nb < plane_size / sizeof(uint8_t); nb++) {
//                            for (int ch = 0; ch < ctx->channels; ch++) {
//                                out[write_p] = (((uint8_t *) frame->extended_data[0])[nb] - 127) * std::numeric_limits<short>::max() / 127;
//                                write_p++;
//                            }
//                        }
//                        ao_play(adevice, (char *) samples, (plane_size / sizeof(uint8_t)) * sizeof(uint16_t) * ctx->channels);
//                        break;
//                    case AV_SAMPLE_FMT_U8:
//                        for (int nb = 0; nb < plane_size / sizeof(uint8_t); nb++) {
//                            out[nb] = static_cast<short>((((uint8_t *) frame->extended_data[0])[nb] - 127) * std::numeric_limits<short>::max() / 127);
//                        }
//                        ao_play(adevice, (char *) samples, (plane_size / sizeof(uint8_t)) * sizeof(uint16_t));
//                        break;
//                    default:
//                        DBG("PCM type not supported");
//                }
//            } else {
//                DBG("frame failed");
//            }
//        }
//        av_free_packet(&packet);
//    }
//    ao_shutdown();
//
//    avcodec_close(ctx);
//    av_close_input_file(container);
//    delete buffer;
//    delete samples;
//    return 0;
//}


