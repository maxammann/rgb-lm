#include <ao/ao.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavresample/avresample.h"
#include "libavutil/opt.h"
#include "../audio.h"

#define CLEAN() clean(device, frame, fmt_ctx, dec_ctx, resample);

static void clean(ao_device *device,
                  AVFrame *frame,
                  AVFormatContext *fmt_ctx,
                  AVCodecContext *dec_ctx,
                  AVAudioResampleContext *resample) {

    avcodec_free_frame(&frame);

    avcodec_close(dec_ctx);
    avformat_close_input(&fmt_ctx);

    avresample_close(resample);
    avresample_free(&resample);

    ao_close(device);
    ao_shutdown();
}

static inline void modify_volume(int16_t *stream, int bytes, double volume) {
    size_t length = bytes / sizeof(uint16_t);

    for (int i = 0; i < length; ++i) {
        stream[i] = (int16_t) (stream[i] * volume);
    }
}

int play(char *file_path, double max_vol, brake brake_fn) {
    int ret;

    double delta_vol = (max_vol == 0) ? NAN : 1.0 / max_vol;

    // Packet
    AVPacket packet;
    av_init_packet(&packet);


    // Frame
    AVFrame *frame = avcodec_alloc_frame();


    // Contexts
    AVAudioResampleContext *resample;
    AVFormatContext *fmt_ctx = 0;
    AVCodecContext *dec_ctx = 0;

    int audio_stream_index;

    av_register_all();

    // Find codec and stream
    AVCodec *dec;
    if ((ret = avformat_open_input(&fmt_ctx, file_path, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    if ((av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a audio stream in the input file\n");
        return ret;
    }

    audio_stream_index = ret;
    dec_ctx = fmt_ctx->streams[audio_stream_index]->codec;

    // Open codec
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open audio decoder\n");
        return ret;
    }

    // Setup resampling
    resample = avresample_alloc_context();


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


    // Setup driver
    ao_device *device;
    ao_sample_format format;
    int default_driver;

    ao_initialize();
    default_driver = ao_default_driver_id();

    format.bits = av_get_bytes_per_sample(output_fmt) * 8;
    format.channels = dec_ctx->channels;
    format.rate = sample_rate;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;

    device = ao_open_live(default_driver, &format, NULL);

    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
        return 1;
    }

    struct timespec start_time;

    if (max_vol != 0.0) {
        clock_gettime(CLOCK_REALTIME, &start_time);
    }

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


            if (got_frame) {

                //Normalize the stream by resampling it
                uint8_t *output;
                int out_linesize;
                int out_samples = avresample_get_out_samples(resample, frame->nb_samples);
                av_samples_alloc(&output, &out_linesize, 2, out_samples, output_fmt, 0);

                avresample_convert(resample, &output, out_linesize, out_samples,
                                   frame->data, frame->linesize[0], frame->nb_samples);

                // If we want to slowly increase volume
                if (delta_vol != NAN) {
                    struct timespec current;
                    clock_gettime(CLOCK_REALTIME, &current);
                    double elapsed = fabs(start_time.tv_sec * 10E9 + start_time.tv_nsec - current.tv_sec * 10E9 +
                                          current.tv_nsec) / 10E9;

                    double volume = fmin(VOLUME_FINISHED, delta_vol * elapsed);

                    if (volume == VOLUME_FINISHED) {
                        delta_vol = NAN;
                    } else {
                        int16_t *stream = (int16_t *) output;

                        modify_volume(stream, out_linesize, volume);
                    }
                }


                if (ao_play(device, (char *) output, (uint_32) out_linesize) == 0) {
                    printf("ao_play: failed.\n");
                    CLEAN();
                    break;
                }

                av_freep(&output);
            }
        }

        av_free_packet(&packet);

        if (brake_fn != NULL && brake_fn()) {
            CLEAN();
            break;
        }
    }

    CLEAN();

    return 0;
}


