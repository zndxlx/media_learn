#include "stdio.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "SDL.h"
void play_destroy();
//const char *inputFileName = "../../assert/1280_720.flv"; //输入文件名
const char *inputFileName = "../../assert/Titanic.ts";
typedef struct
{
    AVFormatContext *pFormatCtx;
    AVPacket *packet;
    AVFrame *pFrame;
    int videoindex;
    uint8_t *video_dst_data[4];
    int video_dst_linesize[4];
    int video_dst_bufsize;
    AVCodecContext *pVCodecCtx;
    AVCodec *pVCodec;
} VPlayCxt;

typedef struct
{
    int screen_w;
    int screen_h;
    SDL_Window *screen;
    SDL_Renderer *sdlRenderer;
    SDL_Texture *sdlTexture;
    SDL_Rect sdlRect;
} SdlCxt;

VPlayCxt vCtx;
SdlCxt sdlCtx;
void decodeV(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt)
{

    int ret = avcodec_send_packet(dec_ctx, pkt); //将pkt 里面数据发送到解码器
    if (ret < 0)
    {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec_ctx, frame); //从解码器读取解码后的数据
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }
        if (ret == 0)
        {
            // av_image_copy(vCtx.video_dst_data, vCtx.video_dst_linesize,
            //               (const uint8_t **)(frame->data), frame->linesize,
            //               dec_ctx->pix_fmt, dec_ctx->width, dec_ctx->height);

            // SDL_UpdateTexture(sdlCtx.sdlTexture, NULL, vCtx.video_dst_data[0], vCtx.video_dst_linesize[0]);
            // SDL_RenderClear(sdlCtx.sdlRenderer);
            // SDL_RenderCopy(sdlCtx.sdlRenderer, sdlCtx.sdlTexture, NULL, NULL);
            // SDL_RenderPresent(sdlCtx.sdlRenderer);

            av_image_copy(vCtx.video_dst_data, vCtx.video_dst_linesize,
                          (const uint8_t **)(frame->data), frame->linesize,
                          dec_ctx->pix_fmt, dec_ctx->width, dec_ctx->height);
            SDL_RenderClear(sdlCtx.sdlRenderer);
            SDL_UpdateTexture(sdlCtx.sdlTexture, NULL, vCtx.video_dst_data[0], vCtx.video_dst_linesize[0]);

            SDL_RenderCopy(sdlCtx.sdlRenderer, sdlCtx.sdlTexture, NULL, NULL);
            SDL_RenderPresent(sdlCtx.sdlRenderer);
        }
    }
}

//play_thread

int play_init()
{
    memset(&vCtx, 0, sizeof(vCtx));

    vCtx.pFormatCtx = avformat_alloc_context();
    AVFormatContext *pFormatCtx = vCtx.pFormatCtx;
    if (avformat_open_input(&pFormatCtx, inputFileName, NULL, NULL) != 0)
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }

    if (avformat_find_stream_info(vCtx.pFormatCtx, NULL) < 0)
    {
        printf("Couldn't find stream information.\n");
        play_destroy();
        return -1;
    }
    int videoindex = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }

    if (videoindex == -1)
    {
        printf("Didn't find a video stream.\n");
        return -1;
    }
    vCtx.videoindex = videoindex;
    vCtx.pVCodecCtx = pFormatCtx->streams[videoindex]->codec;
    vCtx.pVCodec = avcodec_find_decoder(vCtx.pVCodecCtx->codec_id);
    if (vCtx.pVCodec == NULL)
    {
        printf("Codec not found.\n");
        play_destroy();
        return -1;
    }
    if (avcodec_open2(vCtx.pVCodecCtx, vCtx.pVCodec, NULL) < 0)
    {
        printf("Could not open codec.\n");
        play_destroy();
        return -1;
    }

    /* allocate image where the decoded image will be put, 指定alig为1，表示1字节对齐，也就是不对齐 */
    int ret = av_image_alloc(vCtx.video_dst_data, vCtx.video_dst_linesize,
                             vCtx.pVCodecCtx->width, vCtx.pVCodecCtx->height, vCtx.pVCodecCtx->pix_fmt, 1);
    if (ret < 0)
    {
        printf("Could not allocate raw video buffer\n");
        return -1;
    }
    vCtx.video_dst_bufsize = ret;

    printf("nb_streams=%d, duration=%d, bit_rate=%d\n", pFormatCtx->nb_streams, pFormatCtx->duration, pFormatCtx->bit_rate);
    printf("fomat name=%s, long_name=%s\n", pFormatCtx->iformat->name, pFormatCtx->iformat->long_name);
    printf("width=%d, height=%d\n", vCtx.pVCodecCtx->width, vCtx.pVCodecCtx->height);

    vCtx.packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    vCtx.pFrame = av_frame_alloc();
    return 0;
}

void play_destroy()
{
    if (vCtx.video_dst_data[0])
        av_freep(&vCtx.video_dst_data[0]);
    if (vCtx.pFrame)
        av_frame_free(&vCtx.pFrame);
    if (vCtx.packet)
        av_free_packet(vCtx.packet);
    if (vCtx.pVCodecCtx)
        avcodec_close(vCtx.pVCodecCtx);
    if (vCtx.pFormatCtx)
        avformat_close_input(&(vCtx.pFormatCtx));
}

uint32_t tb(uint32_t interval, void *param)
{
    //printf("now %d\n", SDL_GetTicks());
    while (1)
    {
        if (av_read_frame(vCtx.pFormatCtx, vCtx.packet) >= 0)
        {
            if (vCtx.packet->stream_index == vCtx.videoindex)
            {
                decodeV(vCtx.pVCodecCtx, vCtx.pFrame, vCtx.packet);
                break;
            }
        }
        else
        {
            break;
        }
    }

    return interval;
}

int sdl_init()
{
    memset(&sdlCtx, 0, sizeof(sdlCtx));

    int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    if (ret != 0)
    {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    sdlCtx.screen_w = vCtx.pVCodecCtx->width;
    sdlCtx.screen_h = vCtx.pVCodecCtx->height;
    sdlCtx.screen = SDL_CreateWindow("Simplest ffmpeg player's Window",
                                     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                     sdlCtx.screen_w, sdlCtx.screen_h, SDL_WINDOW_OPENGL);

    if (!sdlCtx.screen)
    {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    sdlCtx.sdlRenderer = SDL_CreateRenderer(sdlCtx.screen, -1, 0);
    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    sdlCtx.sdlTexture = SDL_CreateTexture(sdlCtx.sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
                                          vCtx.pVCodecCtx->width, vCtx.pVCodecCtx->height);

    sdlCtx.sdlRect.x = 0;
    sdlCtx.sdlRect.y = 0;
    sdlCtx.sdlRect.w = sdlCtx.screen_w;
    sdlCtx.sdlRect.h = sdlCtx.screen_h;
    return 0;
}

int sdl_destroy()
{
    return 0;
}

int main(int argc, char **argv)
{
    int ret = play_init();
    if (ret != 0)
    {
        printf("play_init failed\n");
        return -1;
    }

    ret = sdl_init();
    if (ret != 0)
    {
        printf("sdl_init failed\n");
        play_destroy();
        return -1;
    }
    printf("aaaaaaaaaaaa\n");
    //初始化定时器
    SDL_TimerID timer = SDL_AddTimer(40, tb, NULL);
    int quit = 0;
    SDL_Event event;
    //处理事件
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                quit = 1;
        }
    }
    SDL_RemoveTimer(timer);
    SDL_Quit();
    printf("hahahhah\n");
    system("pause");
    return 0;
}