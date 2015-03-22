#include <ao/ao.h>
#include <signal.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"


#define BUFFER_SIZE 8192

int cancel_playback;

void on_cancel_playback(int sig) {
    if (sig != SIGINT) {
        return;
    }

    cancel_playback = 1;
    exit(0);
}

static void clean(ao_device *device) {
    ao_close(device);
    ao_shutdown();
}

static AVFormatContext *fmt_ctx;
static AVCodecContext *dec_ctx;

static int audio_stream_index = -1;

static int open_input_file(const char *filename) {
    int ret;
    AVCodec *dec;

    if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    /* select the audio stream */
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a audio stream in the input file\n");
        return ret;
    }
    audio_stream_index = ret;
    dec_ctx = fmt_ctx->streams[audio_stream_index]->codec;

    /* init the audio decoder */
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open audio decoder\n");
        return ret;
    }

    return 0;
}


int play(char *file_path) {
    ao_device *device;
    ao_sample_format format;


    int default_driver;

    signal(SIGINT, on_cancel_playback);


    int ret;
    AVPacket packet;
    AVFrame *frame = avcodec_alloc_frame();
    av_init_packet(&packet);
    av_register_all();

    if (open_input_file(file_path) < 0) return 1;


    ao_initialize();

    default_driver = ao_default_driver_id();

    format.bits = av_get_bytes_per_sample(dec_ctx->sample_fmt) * 8;

    format.channels = dec_ctx->channels;
    format.rate = dec_ctx->sample_rate;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;


    printf("Sample rate: %d\n", dec_ctx->sample_rate);
    printf("Channels: %d\n", dec_ctx->channels);
    printf("bits per sample: %d\n", format.bits);

    device = ao_open_live(default_driver, &format, NULL);

    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
        return 1;
    }

    int buffer_size = 192000 + FF_INPUT_BUFFER_PADDING_SIZE;
    uint8_t *samples[buffer_size];
    int plane_size;

    while (1) {
        if ((ret = av_read_frame(fmt_ctx, &packet)) < 0) {
            break;
        }

        if (packet.stream_index == audio_stream_index) {
            int got_frame = 0;

            ret = avcodec_decode_audio4(dec_ctx, frame, &got_frame, &packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error decoding audio\n");
                continue;
            }

            av_samples_get_buffer_size(&plane_size, dec_ctx->channels, frame->nb_samples, dec_ctx->sample_fmt, 1);


            if (got_frame) {
                int write_p = 0;



                uint16_t *out = (uint16_t *) samples;

                for (int nb = 0; nb < plane_size / sizeof(uint16_t); nb++) {
                    for (int ch = 0; ch < dec_ctx->channels; ch++) {
                        out[write_p] = ((uint16_t *) frame->extended_data[ch])[nb];
                        write_p++;
                    }
                }

                if (ao_play(device,(char *) samples, (plane_size) * dec_ctx->channels) == 0) {
                    printf("ao_play: failed.\n");
                    clean(device);
                    break;
                }

//                if (ao_play(device,(char *) frame->data[0], frame->linesize[0]) == 0) {
//                    printf("ao_play: failed.\n");
//                    clean(device);
//                    break;
//                }
            }
        }

        av_free_packet(&packet);

        if (cancel_playback) {
            clean(device);
            break;
        }
    }

    clean(device);

    return 0;
}


