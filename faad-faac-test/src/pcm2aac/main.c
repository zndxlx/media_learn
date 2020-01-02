#include <stdio.h>
#include "faac.h"
#include "input.h"
//#include "audio.h"

const char *input_file_path = "../../sample.pcm";
const char *output_file_path = "a.aac";

int encode()
{
    pcmfile_t *infile = pcm_open_read(input_file_path, 2, 2, 0, 16000); //格式要和实际的pcm格式对应
    if (infile == NULL)
    {
        goto end;
    }

    FILE *outfile = fopen(output_file_path, "wb");
    if (outfile == NULL)
    {
        goto end;
    }

    //打开编码器
    unsigned long samplesInput = 0;   //解码器返回的期望编码时候传给编码前的采样数量
    unsigned long maxBytesOutput = 0; //每次编码输出最大缓存大小
    faacEncHandle hEncoder = faacEncOpen(infile->samplerate, infile->channels,
                                         &samplesInput, &maxBytesOutput);
    printf("samplesInput=%ld, maxBytesOutput=%ld\n", samplesInput, maxBytesOutput);

    //设置编码前参数
    faacEncConfigurationPtr myFormat = faacEncGetCurrentConfiguration(hEncoder);
    myFormat->aacObjectType = LOW;
    myFormat->mpegVersion = 4;
    myFormat->useTns = 0;
    myFormat->shortctl = 0;

    myFormat->bandWidth = 0;                  //编码带宽
    myFormat->bitRate = 0;                    //设置码率
    myFormat->outputFormat = ADTS_STREAM;     //控制编码输出有adts头信息
    myFormat->inputFormat = FAAC_INPUT_FLOAT; //设置输入的pcm格式

    if (!faacEncSetConfiguration(hEncoder, myFormat))
    {
        fprintf(stderr, "Unsupported output format!\n");
        goto end;
    }

    //从文件读取数据的缓存
    float *pcmbuf = (float *)malloc(samplesInput * sizeof(float));
    //解码输出的缓存
    unsigned char *bitbuf = (unsigned char *)malloc(maxBytesOutput * sizeof(unsigned char));
    while (1)
    {
        //samplesInput表示期望读取的是采样数量, samplesRead是实际返回的采样数量
        int samplesRead = pcm_read_float32(infile, pcmbuf, samplesInput);

        int bytesWritten = faacEncEncode(hEncoder,
                                         (int32_t *)pcmbuf,
                                         samplesRead, bitbuf, maxBytesOutput);

        if (!samplesRead && !bytesWritten) //结束标志
        {
            break;
        }
        if (bytesWritten < 0)
        {
            fprintf(stderr, "faacEncEncode() failed\n");
            break;
        }

        if (bytesWritten > 0)
        {
            fwrite(bitbuf, 1, bytesWritten, outfile);
        }
    }
end:
    //关闭编码器
    if (hEncoder != NULL)
    {
        faacEncClose(hEncoder);
    }
    if (infile != NULL)
    {
        //关闭输入文件
        pcm_close(infile);
    }

    if (outfile != NULL)
    {
        fclose(outfile);
    }
    return 0;
}

int main(void)
{
    encode();
}