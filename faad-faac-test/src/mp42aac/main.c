

#include <time.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "mp4read.h"
#include <neaacdec.h>
#include <getopt.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

static int adts_sample_rates[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0};

static int FindAdtsSRIndex(int sr)
{
    int i;

    for (i = 0; i < 16; i++)
    {
        if (sr == adts_sample_rates[i])
            return i;
    }
    return 16 - 1;
}

static unsigned char *MakeAdtsHeader(int *dataSize, NeAACDecFrameInfo *hInfo, int old_format)
{
    unsigned char *data;
    int profile = (hInfo->object_type - 1) & 0x3;
    int sr_index = ((hInfo->sbr == SBR_UPSAMPLED) || (hInfo->sbr == NO_SBR_UPSAMPLED)) ? FindAdtsSRIndex(hInfo->samplerate / 2) : FindAdtsSRIndex(hInfo->samplerate);
    int skip = (old_format) ? 8 : 7;
    int framesize = skip + hInfo->bytesconsumed;

    if (hInfo->header_type == ADTS)
        framesize -= skip;

    *dataSize = 7;

    data = malloc(*dataSize * sizeof(unsigned char));
    memset(data, 0, *dataSize * sizeof(unsigned char));

    data[0] += 0xFF; /* 8b: syncword */

    data[1] += 0xF0; /* 4b: syncword */
    /* 1b: mpeg id = 0 */
    /* 2b: layer = 0 */
    data[1] += 1; /* 1b: protection absent */

    data[2] += ((profile << 6) & 0xC0);  /* 2b: profile */
    data[2] += ((sr_index << 2) & 0x3C); /* 4b: sampling_frequency_index */
    /* 1b: private = 0 */
    data[2] += ((hInfo->channels >> 2) & 0x1); /* 1b: channel_configuration */

    data[3] += ((hInfo->channels << 6) & 0xC0); /* 2b: channel_configuration */
    /* 1b: original */
    /* 1b: home */
    /* 1b: copyright_id */
    /* 1b: copyright_id_start */
    data[3] += ((framesize >> 11) & 0x3); /* 2b: aac_frame_length */

    data[4] += ((framesize >> 3) & 0xFF); /* 8b: aac_frame_length */

    data[5] += ((framesize << 5) & 0xE0); /* 3b: aac_frame_length */
    data[5] += ((0x7FF >> 6) & 0x1F);     /* 5b: adts_buffer_fullness */

    data[6] += ((0x7FF << 2) & 0x3F); /* 6b: adts_buffer_fullness */
    /* 2b: num_raw_data_blocks */

    return data;
}

static int decodeMP4file(char *mp4filePath, char *adtsFilePath)
{

    //void *sample_buffer;
    unsigned char *adtsData;
    int adtsDataSize;

    unsigned long samplerate;
    unsigned char channels;
    unsigned int framesize;
    NeAACDecFrameInfo frameInfo;
    mp4AudioSpecificConfig mp4ASC;
    mp4config.verbose.header = 1;
    mp4config.verbose.tags = 1;

    if (mp4read_open(mp4filePath))
    {
        /* unable to open file */
        printf("Error opening file: %s\n", mp4filePath);
        return 1;
    }

    //打开解码器
    NeAACDecHandle hDecoder = 0;
    hDecoder = NeAACDecOpen();
    NeAACDecConfigurationPtr config = NeAACDecGetCurrentConfiguration(hDecoder);

    config->outputFormat = FAAD_FMT_16BIT; //设置解码后的pcm格式
    config->defObjectType = LC;
    config->defSampleRate = 44100; //real samplerate/2
    config->dontUpSampleImplicitSBR = 0;
    NeAACDecSetConfiguration(hDecoder, config);
    //int bread = 0;

    if (NeAACDecInit2(hDecoder, mp4config.asc.buf, mp4config.asc.size,
                      &samplerate, &channels) < 0)
    {
        /* If some error initializing occured, skip the file */
        printf("Error initializing decoder library.\n");
        NeAACDecClose(hDecoder);
        mp4read_close();
        return 1;
    }

    FILE *adtsFile = fopen(adtsFilePath, "wb");
    if (adtsFile == NULL)
    {
        printf("Error opening file: %s\n", adtsFilePath);
        return 1;
    }

    framesize = 1024;
    // useAacLength = 0;
    //decoded = 0;

    if (mp4config.asc.size)
    {
        if (NeAACDecAudioSpecificConfig(mp4config.asc.buf, mp4config.asc.size, &mp4ASC) >= 0)
        {
            if (mp4ASC.frameLengthFlag == 1)
                framesize = 960;
            if (mp4ASC.sbr_present_flag == 1)
                framesize *= 2;
        }
    }

    /* print some mp4 file info */
    printf("%s file info:\n\n", mp4filePath);
    {
        //char *tag = NULL;
        //int k, j;
        char *ot[6] = {"NULL", "MAIN AAC", "LC AAC", "SSR AAC", "LTP AAC", "HE AAC"};
        float seconds;
        seconds = (float)mp4config.samples / (float)mp4ASC.samplingFrequency;

        printf("%s\t%.3f secs, %d ch, %d Hz\n\n", ot[(mp4ASC.objectTypeIndex > 5) ? 0 : mp4ASC.objectTypeIndex],
               seconds, mp4ASC.channelsConfiguration, mp4ASC.samplingFrequency);
    }

    int startSampleId = 0;
    //int sampleId = 0;

    mp4read_seek(startSampleId);
    for (int sampleId = startSampleId; sampleId < mp4config.frame.ents; sampleId++)
    {

        if (mp4read_frame())
            break;

        NeAACDecDecode(hDecoder, &frameInfo, mp4config.bitbuf.data, mp4config.bitbuf.size);

        if (1)
        {
            adtsData = MakeAdtsHeader(&adtsDataSize, &frameInfo, 0);

            /* write the adts header */
            fwrite(adtsData, 1, adtsDataSize, adtsFile);

            fwrite(mp4config.bitbuf.data, 1, frameInfo.bytesconsumed, adtsFile);
        }
    }

    mp4read_close();

    fclose(adtsFile);

    return 0;
}

int main(int argc, char *argv[])
{
    return decodeMP4file("a.mp4", "b.aac");
}