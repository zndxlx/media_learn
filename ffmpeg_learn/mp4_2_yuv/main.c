#include "stdio.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"

//提取mp4文件中的视频

const char *inputFileName = "1280_720.mp4";  //输入文件名
const char *outputFileName = "a.yuv"; //输出文件名
static AVFormatContext *fmtCtx = NULL;
static AVFrame *frame = NULL;
static AVPacket pkt;
//FILE *fin;
static FILE *fout = NULL;
static int video_frame_count = 0;

static uint8_t *video_dst_data[4] = {NULL};
static int video_dst_linesize[4];
static int video_dst_bufsize;

//1、avformat_open_input, avformat_find_stream_info 初始化 AVCodecContext
//2、av_find_best_stream 根据fmt的中信息查找流
//3、根据流类型查找编码类型AVCodec
//4、申请并打开编码器上下文
//5、从文件中读取流并判断流类型，如果是视频就将器解码并写入文件

int init_para(void)
{
    int ret = 0;
    ret = avformat_open_input(&fmtCtx, inputFileName, NULL, NULL);
    if (ret < 0)
    {
        printf("avformat_open_input failed %d\n", ret);
        return -1;
    }

    ret = avformat_find_stream_info(fmtCtx, NULL);
    if (ret < 0)
    {
        printf("avformat_find_stream_info failed %d\n", ret);
        return -1;
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        printf("Could not allocate frame\n");
        return -1;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    fout = fopen(outputFileName, "wb");
    if (!fout)
    {
        printf("Could not open destination file %s\n", outputFileName);
        return -1;
    }

    return 0;
}

static int decode_packet(AVCodecContext *video_dec_ctx, int video_stream_idx, int *got_frame)
{
    int ret = 0;
    int decoded = pkt.size;

    *got_frame = 0;

    if (pkt.stream_index == video_stream_idx)
    {
        /* decode video frame */
        ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
        if (ret < 0)
        {
            printf("Error decoding video frame (%s)\n", av_err2str(ret));
            return ret;
        }

        if (*got_frame)
        {
            /* copy decoded frame to destination buffer:
             * this is required since rawvideo expects non aligned data */
            av_image_copy(video_dst_data, video_dst_linesize,
                          (const uint8_t **)(frame->data), frame->linesize,
                          video_dec_ctx->pix_fmt, video_dec_ctx->width, video_dec_ctx->height);

            /* write to rawvideo file */
            fwrite(video_dst_data[0], 1, video_dst_bufsize, fout);
        }
    }

    return decoded;
}

int main()
{
    int ret = 0;
    ret = init_para();
    if (ret != 0)
    {
        return -1;
    }

    int videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoStreamIndex < 0)
    {
        printf("av_find_best_stream failed %d\n", videoStreamIndex);
        return -1;
    }

    AVStream *stream = fmtCtx->streams[videoStreamIndex];

    AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (dec == NULL)
    {
        printf("avcodec_find_decoder failed, codec_id=%d\n", stream->codecpar->codec_id);
        return -1;
    }

    AVCodecContext *decCtx = avcodec_alloc_context3(dec);
    if (decCtx == NULL)
    {
        printf("avcodec_alloc_context3 failed\n");
        return -1;
    }

    /* Copy codec parameters from input stream to output codec context */
    if ((ret = avcodec_parameters_to_context(decCtx, stream->codecpar)) < 0)
    {
        printf("Failed to copy odec parameters to decoder context\n");
        return -1;
    }

    ret = avcodec_open2(decCtx, dec, NULL);
    if (ret != 0)
    {
        printf("avcodec_alloc_context3 failed\n");
        return -1;
    }

    /* allocate image where the decoded image will be put */
    ret = av_image_alloc(video_dst_data, video_dst_linesize,
                         decCtx->width, decCtx->height, decCtx->pix_fmt, 1);
    if (ret < 0)
    {
        printf("Could not allocate raw video buffer\n");
        return -1;
    }
    video_dst_bufsize = ret;

    av_dump_format(fmtCtx, 0, inputFileName, 0);

    int got_frame = 0;
    while (av_read_frame(fmtCtx, &pkt) >= 0)
    {
        AVPacket orig_pkt = pkt;
        do
        {
            ret = decode_packet(decCtx, videoStreamIndex, &got_frame);
            if (ret < 0)
                break;
            if (got_frame)
            {
                printf("video_frame n:%d coded_n:%d\n", video_frame_count++, frame->coded_picture_number);
            }
            pkt.data += ret;
            pkt.size -= ret;
        } while (pkt.size > 0);
        av_packet_unref(&orig_pkt);
    }

    /* flush cached frames */
    pkt.data = NULL;
    pkt.size = 0;
    do
    {
        decode_packet(decCtx, videoStreamIndex, &got_frame);
        if (got_frame)
        {
            printf("cache video_frame n:%d coded_n:%d\n", video_frame_count++, frame->coded_picture_number);
        }
    } while (got_frame);

    printf("Demuxing finieshed.\n");


    avcodec_free_context(&decCtx);
    avformat_close_input(&fmtCtx);
    if (fout != NULL)
        fclose(fout);

    av_frame_free(&frame);
    av_free(video_dst_data[0]);
}
