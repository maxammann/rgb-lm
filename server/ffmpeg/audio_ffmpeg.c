#include <ao/ao.h>
#include <signal.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavresample/avresample.h"
#include "libavutil/opt.h"
#include "libavfilter/avfilter.h"

#define CLEAN() clean(device, frame, fmt_ctx, dec_ctx, resample);

int cancel_playback;

void on_cancel_playback(int sig) {
    if (sig != SIGINT) {
        return;
    }

    cancel_playback = 1;
    exit(0);
}

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

AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;

static int init_filters(const char *filters_descr,AVCodecContext *dec_ctx,  AVStream *stream)
{
    char args[512];
    int ret = 0;
    AVFilter *abuffersrc  = avfilter_get_by_name("abuffer");
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    static const enum AVSampleFormat out_sample_fmts[] = { AV_SAMPLE_FMT_S16, -1 };
    static const int64_t out_channel_layouts[] = { AV_CH_LAYOUT_MONO, -1 };
    static const int out_sample_rates[] = { 8000, -1 };
    const AVFilterLink *outlink;
    AVRational time_base = stream->time_base;

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    if (!dec_ctx->channel_layout)
        dec_ctx->channel_layout = av_get_default_channel_layout(dec_ctx->channels);
    snprintf(args, sizeof(args),
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
            time_base.num, time_base.den, dec_ctx->sample_rate,
            av_get_sample_fmt_name(dec_ctx->sample_fmt), dec_ctx->channel_layout);
    ret = avfilter_graph_create_filter(&buffersrc_ctx, abuffersrc, "in",
            args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
        goto end;
    }

    /* buffer audio sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, abuffersink, "out",
            NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "sample_fmts", out_sample_fmts, -1,
            AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "channel_layouts", out_channel_layouts, -1,
            AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "sample_rates", out_sample_rates, -1,
            AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
            &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

    /* Print summary of the sink buffer
     * Note: args buffer is reused to store channel layout string */
    outlink = buffersink_ctx->inputs[0];
    av_get_channel_layout_string(args, sizeof(args), -1, outlink->channel_layout);
    av_log(NULL, AV_LOG_INFO, "Output: srate:%dHz fmt:%s chlayout:%s\n",
            (int)outlink->sample_rate,
            (char *)av_x_if_null(av_get_sample_fmt_name(outlink->format), "?"),
            args);

    end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}


int play(char *file_path) {
    int ret;

    signal(SIGINT, on_cancel_playback);


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

    init_filters("volume=0.5", dec_ctx, fmt_ctx->streams[audio_stream_index]);


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

//    printf("Sample rate: %d\n", dec_ctx->sample_rate);
//    printf("Channels: %d\n", dec_ctx->channels);
//    printf("bits per sample: %d\n", format.bits);

    device = ao_open_live(default_driver, &format, NULL);

    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
        return 1;
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
                uint8_t *output;
                int out_linesize;
                int out_samples = avresample_get_out_samples(resample, frame->nb_samples);
                av_samples_alloc(&output, &out_linesize, 2, out_samples, output_fmt, 0);

                avresample_convert(resample, &output, out_linesize, out_samples,
                        frame->data, frame->linesize[0], frame->nb_samples);


                if (ao_play(device, (char *) output, (uint_32) out_linesize) == 0) {
                    printf("ao_play: failed.\n");
                    CLEAN();
                    break;
                }

                av_freep(&output);
            }
        }

        av_free_packet(&packet);

        if (cancel_playback) {
            CLEAN();
            break;
        }
    }

    CLEAN();

    return 0;
}


