#include <stdio.h>
#include "neaacdec.h"
#include "file_reader.h"
#include "audio.h"

#define MAX_CHANNELS 6 /* make this higher to support files with \
                          more channels */

void printbuf(unsigned char *buf, int l)
{
    int i = 0;
    int line = l / 8;
    for (i = 0; i < line; i++)
    {
        printf("%02x %02x %02x %02x %02x %02x %02x %02x\n", buf[i * 8 + 0],
               buf[i * 8 + 1], buf[i * 8 + 2], buf[i * 8 + 3], buf[i * 8 + 4],
               buf[i * 8 + 5], buf[i * 8 + 6], buf[i * 8 + 7]);
    }
    return;
}

static void print_frame_info(NeAACDecFrameInfo *frameInfo)
{
    printf("frameInfo bytesconsumed=%d, header_type=%lu, channels=%d, sbr=%d, samplerate=%lu\n",
           frameInfo->header_type,
           frameInfo->bytesconsumed,
           frameInfo->channels,
           frameInfo->sbr,
           frameInfo->samplerate);
}

int decodeAACfile()
{
    file_reader *reader = file_reader_create("../../sample.aac", FAAD_MIN_STREAMSIZE * MAX_CHANNELS);
    if (reader == NULL)
    {
        printf("file_reader_create failed");
        return -1;
    }

    //打开解码器
    NeAACDecHandle hDecoder = 0;
    hDecoder = NeAACDecOpen();
    /* Set the default object type and samplerate */
    /* This is useful for RAW AAC files */
    NeAACDecConfigurationPtr config = NeAACDecGetCurrentConfiguration(hDecoder);

    config->outputFormat = FAAD_FMT_16BIT; //设置解码后的pcm格式
    // config->downMatrix = downMatrix;
    //config->useOldADTSFormat = old_format;
    // config->defSampleRate = 16000;
    config->defObjectType = LC;
    config->defSampleRate = 44100; //real samplerate/2
    config->dontUpSampleImplicitSBR = 0;
    unsigned char r = NeAACDecSetConfiguration(hDecoder, config);
    printf("r=%d.\n", r);
    //初始化解码器
    int bread = 0;
    unsigned long samplerate;
    unsigned char channels;

    // char *tmpBuf = malloc(reader->bytes_into_buffer);
    // memcpy(tmpBuf, reader->buffer, reader->bytes_into_buffer);

    if ((bread = NeAACDecInit(hDecoder, reader->buffer,
                              reader->bytes_into_buffer, &samplerate, &channels)) < 0)
    {
        /* If some error initializing occured, skip the file */
        printf("Error initializing decoder library.\n");
        NeAACDecClose(hDecoder);
        file_reader_destroy(reader);
        return 1;
    }
    printf("samplerate=%lu, channels=%d, bread=%d\n", samplerate, channels, bread);
    if (bread > 0)
    {
        file_reader_advance_buffer(reader, bread);
        file_reader_fill_buffer(reader);
    }

    NeAACDecFrameInfo frameInfo;
    void *sample_buffer = NULL;
    int first_time = 1;
    int count = 0;
    audio_file *aufile;
    do
    {
        count++;

        //printbuf(reader->buffer, 80);
        sample_buffer = NeAACDecDecode(hDecoder, &frameInfo,
                                       reader->buffer, reader->bytes_into_buffer);

        /* update buffer indices */
        printf("count=%d, bytesconsumed=%lu\n", count, frameInfo.bytesconsumed);
        file_reader_advance_buffer(reader, frameInfo.bytesconsumed);

        if (frameInfo.error > 0)
        {
            printf("NeAACDecDecode Error: %s, sample_buffer=%p\n",
                   NeAACDecGetErrorMessage(frameInfo.error), sample_buffer);
        }

        /* open the sound file now that the number of channels are known */
        if (first_time && !frameInfo.error)
        {
            /* print some channel info */
            print_frame_info(&frameInfo);
            aufile = open_audio_file("a.pcm", frameInfo.samplerate, frameInfo.channels,
                                     FAAD_FMT_16BIT);

            //保存为文件后，设置采样率为frameInfo.samplerate，16bit小端 签字的播放
            first_time = 0;
        }

        if ((frameInfo.error == 0) && (frameInfo.samples > 0))
        {
            if (write_audio_file(aufile, sample_buffer, frameInfo.samples, 0) == 0)
                break;
        }

        /* fill buffer */
        file_reader_fill_buffer(reader);

        if (reader->bytes_into_buffer == 0)
            sample_buffer = NULL; /* to make sure it stops now */

    } while (sample_buffer != NULL);

    //NeAACDecClose(hDecoder);
    file_reader_destroy(reader);
    return 0;
}

int main(void)
{
    decodeAACfile();
}