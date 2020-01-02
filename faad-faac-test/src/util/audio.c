
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <neaacdec.h>
#include <stdint.h>

#include "audio.h"

audio_file *open_audio_file(char *infile, int samplerate, int channels,
                            int outputFormat)
{
    audio_file *aufile = malloc(sizeof(audio_file));

    aufile->outputFormat = outputFormat;

    aufile->samplerate = samplerate;
    aufile->channels = channels;
    aufile->total_samples = 0;

    switch (outputFormat)
    {
    case FAAD_FMT_16BIT:
        aufile->bits_per_sample = 16;
        break;
    case FAAD_FMT_24BIT:
        aufile->bits_per_sample = 24;
        break;
    case FAAD_FMT_32BIT:
    case FAAD_FMT_FLOAT:
        aufile->bits_per_sample = 32;
        break;
    default:
        if (aufile)
            free(aufile);
        return NULL;
    }

    aufile->sndfile = fopen(infile, "wb");

    if (aufile->sndfile == NULL)
    {
        if (aufile)
            free(aufile);
        return NULL;
    }

    return aufile;
}

int write_audio_file(audio_file *aufile, void *sample_buffer, int samples, int offset)
{
    char *buf = (char *)sample_buffer;
    switch (aufile->outputFormat)
    {
    case FAAD_FMT_16BIT:
        return write_audio_16bit(aufile, buf + offset * 2, samples);
    case FAAD_FMT_24BIT:
        return write_audio_24bit(aufile, buf + offset * 4, samples);
    case FAAD_FMT_32BIT:
        return write_audio_32bit(aufile, buf + offset * 4, samples);
    case FAAD_FMT_FLOAT:
        return write_audio_float(aufile, buf + offset * 4, samples);
    default:
        return 0;
    }

    return 0;
}

void close_audio_file(audio_file *aufile)
{

    fclose(aufile->sndfile);

    if (aufile)
        free(aufile);
}

int write_audio_16bit(audio_file *aufile, void *sample_buffer,
                      unsigned int samples)
{
    int ret;
    //unsigned int i;
    //short *sample_buffer16 = (short *)sample_buffer;
    char *data = malloc(samples * aufile->bits_per_sample / 8);

    aufile->total_samples += samples;

    //控制输出为小端
    // for (i = 0; i < samples; i++)
    // {
    //     data[i * 2] = (char)(sample_buffer16[i] & 0xFF);
    //     data[i * 2 + 1] = (char)((sample_buffer16[i] >> 8) & 0xFF);
    // }

    // ret = fwrite(data, samples, aufile->bits_per_sample / 8, aufile->sndfile);
    ret = fwrite(sample_buffer, samples, aufile->bits_per_sample / 8, aufile->sndfile);
    if (data)
        free(data);

    return ret;
}

int write_audio_24bit(audio_file *aufile, void *sample_buffer,
                      unsigned int samples)
{
    int ret;
    unsigned int i;
    int32_t *sample_buffer24 = (int32_t *)sample_buffer;
    char *data = malloc(samples * aufile->bits_per_sample / 8);

    aufile->total_samples += samples;

    for (i = 0; i < samples; i++)
    {
        data[i * 3] = (char)(sample_buffer24[i] & 0xFF);
        data[i * 3 + 1] = (char)((sample_buffer24[i] >> 8) & 0xFF);
        data[i * 3 + 2] = (char)((sample_buffer24[i] >> 16) & 0xFF);
    }

    ret = fwrite(data, samples, aufile->bits_per_sample / 8, aufile->sndfile);

    if (data)
        free(data);

    return ret;
}

int write_audio_32bit(audio_file *aufile, void *sample_buffer,
                      unsigned int samples)
{
    int ret;
    unsigned int i;
    int32_t *sample_buffer32 = (int32_t *)sample_buffer;
    char *data = malloc(samples * aufile->bits_per_sample / 8);

    aufile->total_samples += samples;

    for (i = 0; i < samples; i++)
    {
        data[i * 4] = (char)(sample_buffer32[i] & 0xFF);
        data[i * 4 + 1] = (char)((sample_buffer32[i] >> 8) & 0xFF);
        data[i * 4 + 2] = (char)((sample_buffer32[i] >> 16) & 0xFF);
        data[i * 4 + 3] = (char)((sample_buffer32[i] >> 24) & 0xFF);
    }

    ret = fwrite(data, samples, aufile->bits_per_sample / 8, aufile->sndfile);

    if (data)
        free(data);

    return ret;
}

int write_audio_float(audio_file *aufile, void *sample_buffer,
                      unsigned int samples)
{
    int ret;
    unsigned int i;
    float *sample_buffer_f = (float *)sample_buffer;
    unsigned char *data = malloc(samples * aufile->bits_per_sample / 8);

    aufile->total_samples += samples;

    for (i = 0; i < samples; i++)
    {
        int exponent, mantissa, negative = 0;
        float in = sample_buffer_f[i];

        data[i * 4] = 0;
        data[i * 4 + 1] = 0;
        data[i * 4 + 2] = 0;
        data[i * 4 + 3] = 0;
        if (in == 0.0)
            continue;

        if (in < 0.0)
        {
            in *= -1.0;
            negative = 1;
        }
        in = (float)frexp(in, &exponent);
        exponent += 126;
        in *= (float)0x1000000;
        mantissa = (((int)in) & 0x7FFFFF);

        if (negative)
            data[i * 4 + 3] |= 0x80;

        if (exponent & 0x01)
            data[i * 4 + 2] |= 0x80;

        data[i * 4] = mantissa & 0xFF;
        data[i * 4 + 1] = (mantissa >> 8) & 0xFF;
        data[i * 4 + 2] |= (mantissa >> 16) & 0x7F;
        data[i * 4 + 3] |= (exponent >> 1) & 0x7F;
    }

    ret = fwrite(data, samples, aufile->bits_per_sample / 8, aufile->sndfile);

    if (data)
        free(data);

    return ret;
}
