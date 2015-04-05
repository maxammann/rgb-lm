#include <ao/ao.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavresample/avresample.h"
#include "libavutil/opt.h"
#include "audio.h"

#define CLEAN() clean(device, frame, fmt_ctx, dec_ctx, resample);

static void clean(ao_device *device,
                  AVFrame *frame,
                  AVFormatContext *fmt_ctx,
                  AVCodecContext *dec_ctx,
                  AVAudioResampleContext *resample) {

    if (frame != 0) avcodec_free_frame(&frame);


    if (dec_ctx != 0) avcodec_close(dec_ctx);
    if (fmt_ctx != 0) avformat_close_input(&fmt_ctx);

    if (resample != 0) avresample_close(resample);
    if (resample != 0) avresample_free(&resample);

    if (device != 0) ao_close(device);
    ao_shutdown();
}

static inline void modify_volume(int16_t *stream, int bytes, double volume) {
    size_t length = bytes / sizeof(uint16_t);

    for (int i = 0; i < length; ++i) {
        stream[i] = (int16_t) (stream[i] * volume);
    }
}

int audio_play_default(char *file_path, double seconds, brake brake_fn) {
    return audio_play(file_path, seconds, 1.0, brake_fn);
}

void audio_init() {
    ao_initialize();
    av_register_all();
}

int open_file(char *file_path, AVFormatContext **fmt_ctx, AVCodecContext **dec_ctx) {
    int audio_stream_index;
    AVCodec *codec;

    // Find codec and stream
    if (avformat_open_input(fmt_ctx, file_path, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return -1;
    }

    if (avformat_find_stream_info(*fmt_ctx, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return -1;
    }

    if ((audio_stream_index = av_find_best_stream(*fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a audio stream in the input file\n");
        return -1;
    }

    *dec_ctx = (*fmt_ctx)->streams[audio_stream_index]->codec;

    // Open codec
    if (avcodec_open2(*dec_ctx, codec, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open audio decoder\n");
        return -1;
    }

    return audio_stream_index;
}

enum AVSampleFormat init_resampling(AVAudioResampleContext **out_resample, AVCodecContext *dec_ctx) {
    AVAudioResampleContext *resample = avresample_alloc_context();

    int64_t layout = av_get_default_channel_layout(dec_ctx->channels);
    int sample_rate = dec_ctx->sample_rate;
    enum AVSampleFormat output_fmt = AV_SAMPLE_FMT_S16;

    av_opt_set_int(resample, "in_channel_layout", layout, 0);
    av_opt_set_int(resample, "out_channel_layout", layout, 0);
    av_opt_set_int(resample, "in_sample_rate", sample_rate, 0);
    av_opt_set_int(resample, "out_sample_rate", sample_rate, 0);
    av_opt_set_int(resample, "in_sample_fmt", dec_ctx->sample_fmt, 0);
    av_opt_set_int(resample, "out_sample_fmt", output_fmt, 0);

    avresample_open(resample);

    *out_resample = resample;

    return output_fmt;
}


int audio_play(char *file_path, double seconds, double max_vol, brake brake_fn) {
    int ret;
    enum {
        SKIP, STATIC, DYNAMIC
    } vol_state;

    double vol;

    if (seconds == 0) {
        if (max_vol == 1.0) {
            vol_state = SKIP;
            vol = 0;
        } else {
            vol_state = STATIC;
            vol = max_vol;
        }

    } else {
        vol_state = DYNAMIC;
        vol = max_vol / seconds;
    }

    ao_device *device = 0;
    ao_sample_format format;
    int default_driver;

    // Packet
    AVPacket packet;
    av_init_packet(&packet);


    // Frame
    AVFrame *frame = avcodec_alloc_frame();

    // Contexts
    AVAudioResampleContext *resample = 0;
    AVFormatContext *fmt_ctx = 0;
    AVCodecContext *dec_ctx = 0;

    int audio_stream_index = open_file(file_path, &fmt_ctx, &dec_ctx);

    if (audio_stream_index < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error opening file\n");
        return audio_stream_index;
    }

    // Setup resampling
    enum AVSampleFormat output_fmt = init_resampling(&resample, dec_ctx);
    int sample_rate = dec_ctx->sample_rate;

    // Setup driver
    default_driver = ao_default_driver_id();

    format.bits = av_get_bytes_per_sample(output_fmt) * 8;
    format.channels = dec_ctx->channels;
    format.rate = sample_rate;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;

    device = ao_open_live(default_driver, &format, NULL);

    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
        CLEAN();
        return 1;
    }

    struct timespec start_time;

    if (vol_state == DYNAMIC) {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
    }

    while (1) {
        if ((av_read_frame(fmt_ctx, &packet)) < 0) {
            break;
        }

        if (packet.stream_index == audio_stream_index) {
            int got_frame = 0;

            ret = avcodec_decode_audio4(dec_ctx, frame, &got_frame, &packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error decoding audio\n");
                continue;
            }


            if (got_frame) {

                //Normalize the stream by resampling it
                uint8_t *output;
                int out_linesize;
                int out_samples = avresample_get_out_samples(resample, frame->nb_samples);
                av_samples_alloc(&output, &out_linesize, 2, out_samples, output_fmt, 0);

                avresample_convert(resample, &output, out_linesize, out_samples,
                                   frame->data, frame->linesize[0], frame->nb_samples);

                if (vol_state == STATIC) {
                    modify_volume((int16_t *) output, out_linesize, vol);
                } else if (vol_state == DYNAMIC) {
                    struct timespec current;
                    clock_gettime(CLOCK_MONOTONIC, &current);
                    double elapsed = fabs(start_time.tv_sec * 10E9 + start_time.tv_nsec - current.tv_sec * 10E9 +
                                          current.tv_nsec) / 10E9;

                    double volume = fmin(max_vol, vol * elapsed);

                    if (volume == 1.0) {
                        vol = SKIP;
                    } else {
                        modify_volume((int16_t *) output, out_linesize, volume);
                    }
                }


                if (ao_play(device, (char *) output, (uint_32) out_linesize) == 0) {
                    printf("ao_play: failed.\n");
                    break;
                }

                av_freep(&output);
            }
        }

        av_free_packet(&packet);

        if (brake_fn != NULL && brake_fn()) {
            break;
        }
    }

    CLEAN();

    return 0;
}


