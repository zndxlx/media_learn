#include <stdio.h>
#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>

int main(int argc, char **argv)
{
    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *tag = NULL;
    int ret;

    // printf("hhhhh\n");

    if (argc != 2)
    {
        printf("usage: %s <input_file>\n"
               "example program to demonstrate the use of the libavformat metadata API.\n"
               "\n",
               argv[0]);
        return 1;
    }

    if ((ret = avformat_open_input(&fmt_ctx, argv[1], NULL, NULL)))
        return ret;

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    else
    {
        av_dump_format(fmt_ctx, 0, argv[1], 0);

        //printf("duration is: %I64d, nb_stream is: %d\n", fmt_ctx->duration, fmt_ctx->nb_streams);
    }

    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        printf("%s=%s\n", tag->key, tag->value);

    avformat_close_input(&fmt_ctx);
    return 0;
}