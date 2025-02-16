


#ifdef __ANDROID__

#include <android/log.h>

#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)
#else
#define LOGE(format, ...)  printf(">_<" format "\n", ##__VA_ARGS__)
#define LOGI(format, ...)  printf("^_^ " format "\n", ##__VA_ARGS__)
#endif

#include <jni.h>



extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/log.h"
#include "libavutil/imgutils.h"
#include <time.h>
}

void custom_log(void *ptr, int level, const char *fmt, va_list vl) {
    FILE *fp = fopen("/data/user/0/com.ffmpeg.code/files/log.txt", "a+");
    if (fp != nullptr) {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}


extern "C"
JNIEXPORT int JNICALL
Java_com_android_ffmpeg_MainActivity_decodeMp4ToYuv(JNIEnv *env, jobject thiz, jstring input_jstr,
                                                    jstring output_jstr) {
    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    const AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    uint8_t *out_buffer;
    AVPacket *packet;
    int y_size;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx;
    FILE *fp_yuv;
    int frame_cnt = 0;
    clock_t time_start, time_finish;
    double time_duration = 0.0;

    char input_str[500] = {0};
    char output_str[500] = {0};
    char info[1000] = {0};

    sprintf(input_str, "%s", (env)->GetStringUTFChars(input_jstr, NULL));
    sprintf(output_str, "%s", (env)->GetStringUTFChars(output_jstr, NULL));

    LOGI("input_str:%s", input_str);
    LOGI("output_str:%s", output_str);

    av_log_set_callback(custom_log);
    avformat_network_init();

    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, input_str, nullptr, nullptr) != 0) {
        LOGE("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        LOGE("Couldn't find stream information.\n");
        return -1;
    }

    videoindex = -1;
    int audio = 0;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        LOGI("总共的帧数 count: %d", pFormatCtx->nb_streams);
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGI("当前音频帧数据 index: %d", i);
        }
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGI("当前视频频帧 index: %d", i);
            videoindex = i;
        }
    }

    if (videoindex == -1) {
        LOGE("Couldn't find a video stream.\n");
        return -1;
    }

    AVCodecParameters *codecpar = pFormatCtx->streams[videoindex]->codecpar;
    pCodec = avcodec_find_decoder(codecpar->codec_id);
    if (pCodec == nullptr) {
        LOGE("Couldn't find Codec.\n");
        return -1;
    }
    // 分配AVCodecContext并设置解码器
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        fprintf(stderr, "无法分配解码器上下文！\n");
        return -1;
    }

    // 将AVCodecParameters复制到AVCodecContext
    if (avcodec_parameters_to_context(pCodecCtx, codecpar) < 0) {
        fprintf(stderr, "将解码器参数复制到解码器上下文失败！\n");
        avcodec_free_context(&pCodecCtx);
        return -1;
    }

    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        LOGE("Couldn't open codec.\n");
        return -1;
    }
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer = (unsigned char *)
            av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                               pCodecCtx->width,
                                               pCodecCtx->height, 1));

    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    LOGI("width:%d, height:%d, pix_fmt:%d", pCodecCtx->width, pCodecCtx->height,
         pCodecCtx->pix_fmt);
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, NULL, NULL, NULL);
    if (!img_convert_ctx) {
        fprintf(stderr, "img_convert_ctx\n");
    }

    sprintf(info, "[Input     ]%s\n", input_str);
    sprintf(info, "%s[Output    ]%s\n", info, output_str);
    sprintf(info, "%s[Format    ]%s\n", info, pFormatCtx->iformat->name);
    sprintf(info, "%s[Codec     ]%s\n", info, pCodecCtx->codec->name);
    sprintf(info, "%s[Resolution]%dx%d\n", info, pCodecCtx->width, pCodecCtx->height);


    LOGI("------  ------ ------ ------ ------ ------ ");

    LOGI("info1 is %s", info);

    fp_yuv = fopen(output_str, "w+");
    if (fp_yuv == nullptr) {
        printf("Cannot open output file.\n");
        return -1;
    }


    int frame_av_cnt = 0;
    time_start = clock();
    packet = (AVPacket *) av_packet_alloc();

    /**
    * 一个packet 一般就是一帧的视频数据或者音频数据.
    */
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index != videoindex) {
            frame_av_cnt++;
            LOGI("当前不是视频画面-跳过...index:%d", packet->stream_index);
            continue;
        }
        if (packet->stream_index == videoindex) {
            LOGI("frame...index:%d", packet->stream_index);
            ret = avcodec_send_packet(pCodecCtx, packet);
            if (ret < 0) {
                LOGE("Decode Error.\n");
                return -1;
            }
            while (avcodec_receive_frame(pCodecCtx, pFrame) >= 0) {
                // 处理解码后的帧（例如显示、保存等）
                LOGI("frame type is %d", pFrame->format);
                sws_scale(img_convert_ctx,
                          (const uint8_t *const *) pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
                fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
                fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
                //Output info
                char pictype_str[10] = {0};
                switch (pFrame->pict_type) {
                    case AV_PICTURE_TYPE_I:
                        sprintf(pictype_str, "I");
                        break;
                    case AV_PICTURE_TYPE_P:
                        sprintf(pictype_str, "P");
                        break;
                    case AV_PICTURE_TYPE_B:
                        sprintf(pictype_str, "B");
                        break;
                    default:
                        sprintf(pictype_str, "Other");
                        break;
                }
                LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
                frame_cnt++;
            }
            //重新初始化.
            av_packet_unref(packet);
        }
    }

    LOGI("Frame av count: %5d ", frame_av_cnt);
    av_packet_free(&packet);

    time_finish = clock();
    time_duration = (double) (time_finish - time_start);

    sprintf(info, "%s[Time      ]%fs\n", info, time_duration / CLOCKS_PER_SEC);

    LOGI("info2 is %s", info);
    LOGI("------  ------ ------ ------ ------ ------ ");

    sws_freeContext(img_convert_ctx);

    fclose(fp_yuv);

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 0;

}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_android_ffmpeg_MainActivity_getFFMpegVersion(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF(av_version_info());
}