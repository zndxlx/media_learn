/**
 * @file
 * libswscale API use example.
 * @example scaling_video.c
 */

#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>

static void fill_yuv_image(uint8_t *data[4], int linesize[4],
                           int width, int height, int frame_index)
{
    int x, y;

    /* Y */
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            data[0][y * linesize[0] + x] = x + y + frame_index * 3;

    /* Cb and Cr */
    for (y = 0; y < height / 2; y++)
    {
        for (x = 0; x < width / 2; x++)
        {
            data[1][y * linesize[1] + x] = 128 + y + frame_index * 2;
            data[2][y * linesize[2] + x] = 64 + x + frame_index * 5;
        }
    }
}

int main(int argc, char **argv)
{
    uint8_t *src_data[4], *dst_data[4];
    int src_linesize[4], dst_linesize[4];
    int src_w = 320, src_h = 240, dst_w, dst_h;
    enum AVPixelFormat src_pix_fmt = AV_PIX_FMT_YUV420P, dst_pix_fmt = AV_PIX_FMT_RGB24;
    const char *dst_size = NULL;
    const char *dst_filename = NULL;
    FILE *dst_file;
    int dst_bufsize;
    struct SwsContext *sws_ctx;
    int i, ret;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s output_file output_size\n"
                        "API example program to show how to scale an image with libswscale.\n"
                        "This program generates a series of pictures, rescales them to the given "
                        "output_size and saves them to an output file named output_file\n."
                        "\n",
                argv[0]);
        exit(1);
    }
    dst_filename = argv[1];
    dst_size = argv[2]; //格式wxh 比如 720x480

    if (av_parse_video_size(&dst_w, &dst_h, dst_size) < 0)
    {
        fprintf(stderr,
                "Invalid size '%s', must be in the form WxH or a valid size abbreviation\n",
                dst_size);
        exit(1);
    }

    dst_file = fopen(dst_filename, "wb");
    if (!dst_file)
    {
        fprintf(stderr, "Could not open destination file %s\n", dst_filename);
        exit(1);
    }

    /* create scaling context   sws_getContext(获取缩放上下文)是简单的方式，
    也可以通过 sws_alloc_context ---》av_opt_set_XXX--》sws_init_context
    这种方式可以设置比较多的参数。
    SWS_BILINEAR 是一种缩放模式，
    */
    sws_ctx = sws_getContext(src_w, src_h, src_pix_fmt,
                             dst_w, dst_h, dst_pix_fmt,
                             SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx)
    {
        fprintf(stderr,
                "Impossible to create scale context for the conversion "
                "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                av_get_pix_fmt_name(src_pix_fmt), src_w, src_h,
                av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h);
        ret = AVERROR(EINVAL);
        goto end;
    }

    /* allocate source and destination image buffers  16 表示16bit对齐 
    src_linesize返回了每个维度需要的内存大小， ret为总大小，src_data指向申请的内存，
    释放时候通过av_freep(&src_data[0])释放
    */
    if ((ret = av_image_alloc(src_data, src_linesize,
                              src_w, src_h, src_pix_fmt, 16)) < 0)
    {
        fprintf(stderr, "Could not allocate source image\n");
        goto end;
    }
    else
    {
        printf("src_linesize[0]=%d, src_linesize[1]=%d,src_linesize[2]=%d\n",
               src_linesize[0], src_linesize[1], src_linesize[2]);
    }

    /* buffer is going to be written to rawvideo file, no alignment 
    1 表示没有对齐
    */
    if ((ret = av_image_alloc(dst_data, dst_linesize,
                              dst_w, dst_h, dst_pix_fmt, 1)) < 0)
    {
        fprintf(stderr, "Could not allocate destination image\n");
        goto end;
    }
    dst_bufsize = ret;

    for (i = 0; i < 100; i++)
    {
        /* generate synthetic video 自己生成的yuv图像*/
        fill_yuv_image(src_data, src_linesize, src_w, src_h, i);

        /* convert to destination format */
        sws_scale(sws_ctx, (const uint8_t *const *)src_data,
                  src_linesize, 0, src_h, dst_data, dst_linesize);

        /* write scaled image to file */
        fwrite(dst_data[0], 1, dst_bufsize, dst_file);
    }

    fprintf(stderr, "Scaling succeeded. Play the output file with the command:\n"
                    "ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
            av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h, dst_filename);

end:
    fclose(dst_file);
    av_freep(&src_data[0]);
    av_freep(&dst_data[0]);
    sws_freeContext(sws_ctx);
    return ret < 0;
}