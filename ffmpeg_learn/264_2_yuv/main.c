#include "stdio.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"

const char *inputFileName = "a.h264"; //输入文件名
const char *outputFileName = "a.yuv";                       //输出文件名
AVCodec *pCodec;                                            //编码器
AVCodecContext *pCodecContext;                              //编码器上下文
AVCodecParserContext *pCodecParserCtx;                      //解析器
AVFrame *frame;
AVPacket pkt;
FILE *fin;
FILE *fout;

int open_files()
{
    //打开输入输出文件
    if ((fin = fopen(inputFileName, "rb")) == NULL)
    {
        printf("cant open the file %s\n", inputFileName);
        return -1;
    }

    if ((fout = fopen(outputFileName, "wb")) == NULL)
    {
        printf("cant open the file %s\n", outputFileName);
        return -1;
    }
    printf("open_files sucess\n");
    return 0;
}

void close_file()
{
    fclose(fin);
    fclose(fout);
}

int open_decoder()
{
    //注册编解码器对象
    avcodec_register_all();

    //初始化AVPacket对象
    av_init_packet(&pkt);

    //根据CODEC_ID查找AVCodec对象
    pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (pCodec == NULL)
    {
        printf("Codec not found\n");
        return -1;
    }

    //根据AVCodec对象分配AVCodecContext
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL)
    {
        printf("Could not allocate video codec context\n");
        return -1;
    }

    if (pCodec->capabilities & AV_CODEC_CAP_TRUNCATED)
        pCodecContext->flags |= AV_CODEC_FLAG_TRUNCATED; // we do not send complete frames

    //根据CODEC_ID初始化AVCodecParserContext对象
    pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264);
    if (NULL == pCodecParserCtx)
    {
        printf("Could not allocate video parser context\n");
        return -1;
    }

    //打开AVCodec对象
    if (avcodec_open2(pCodecContext, pCodec, NULL) != 0)
    {
        printf("Could not open codec\n");
        return -1;
    }

    //分配AVFrame对象
    frame = av_frame_alloc();
    if (frame == NULL)
    {
        printf("Could not allocate video frame\n");
        return -1;
    }

    printf("open_decoder sucess\n");
    return 0;
}

void close_decoder()
{
    av_parser_close(pCodecParserCtx);
    //avcodec_close(pCodecContext);
    //av_free(pCodecContext);
    avcodec_free_context(&pCodecContext);
    
    av_frame_free(&(frame));
}

void write_out_yuv_frame()
{
    uint8_t **pBuf = frame->data;
    int *pStride = frame->linesize;

    for (int color_idx = 0; color_idx < 3; color_idx++)
    {
        int nWidth = color_idx == 0 ? frame->width : frame->width / 2;
        int nHeight = color_idx == 0 ? frame->height : frame->height / 2;
        for (int idx = 0; idx < nHeight; idx++)
        {
            fwrite(pBuf[color_idx], 1, nWidth, fout);
            pBuf[color_idx] += pStride[color_idx];
        }
        fflush(fout);
    }
}


const int INBUF_SIZE = 4096;
int main(void)
{
    //int ret = 0;
    if (open_files() != 0)
    {
        return -1;
    }

    if (open_decoder() != 0)
    {
        return -1;
    }

    char inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    memset(inbuf, 0, sizeof(inbuf));

    int uDataSize = 0;
    char *pDataPtr = NULL;
    int len = 0;
    int got_picture = 0;
    while (1)
    {
        //将码流文件按某长度读入输入缓存区
        uDataSize = fread(inbuf, 1, INBUF_SIZE, fin);

        if (0 == uDataSize)
        {
            break;
        }
        

        pDataPtr = inbuf;

        while (uDataSize > 0)
        {
            //解析缓存区中的数据为AVPacket对象，包含一个NAL Unist的数据，比较迷惑的是要是不完整的数据会保存再哪，可能在static变量，
            //或者pCodecParserCtx和pCodecContext
            len = av_parser_parse2(pCodecParserCtx, pCodecContext,
                                   &(pkt.data), &(pkt.size),
                                   (const uint8_t *)pDataPtr, uDataSize,
                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
            pDataPtr += len;
            uDataSize -= len;

            //参考 av_parser_parse2 说明  如果没有完成pkt.size为0
            if (0 == pkt.size)  
            {
                continue;
            }

            printf("Parse 1 packet. Packet pts: %ld.\n", pkt.pts);

            //根据AVCodecContext的设置，解析AVPacket中的码流，输出到AVFrame
            //avcodec_decode_video2 会判断frame中有没有内存，没有会自己申请，所以第一次会自己申请，后面复用第一次申请的 
            int ret = avcodec_decode_video2(pCodecContext, frame, &got_picture, &(pkt));
            if (ret < 0)
            {
                printf("Decode Error.\n");
                return ret;
            }

            if (got_picture)
            {
                //获得一帧完整的图像，写出到输出文件
                write_out_yuv_frame();
                printf("Succeed to decode 1 frame! Frame pts: %ld\n", frame->pts);
            }
        } //while(uDataSize > 0)
    }

    pkt.data = NULL;
    pkt.size = 0;
    while (1)
    {
        //将编码器中剩余的数据继续输出完
        int ret = avcodec_decode_video2(pCodecContext, frame, &got_picture, &(pkt));
        if (ret < 0)
        {
            printf("Decode Error.\n");
            return ret;
        }

        if (got_picture)
        {
            write_out_yuv_frame();
            printf("Flush Decoder: Succeed to decode 1 frame!\n");
        }
        else
        {
            break;
        }
    } //while(1)

    close_file();
    close_decoder();

    return 0;
}