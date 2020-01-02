
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"

#define SWAP32(x) (((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24))
#define SWAP16(x) (((x & 0xff) << 8) | ((x & 0xff00) >> 8))

#ifdef WORDS_BIGENDIAN
#define UINT32(x) SWAP32(x)
#define UINT16(x) SWAP16(x)
#else
#define UINT32(x) (x)
#define UINT16(x) (x)
#endif

pcmfile_t *pcm_open_read(const char *name, int channels, int samplebytes, int bigendian, int samplerate)
{
  FILE *wave_f;

  //int fmtsize;
  pcmfile_t *sndf;

  if (!(wave_f = fopen(name, "rb")))
  {
    perror(name);
    return NULL;
  }

  if (samplebytes > 4 || samplebytes < 1)
  {
    printf("samplebytes=%d not allow\n", samplebytes);
    return NULL;
  }

  sndf = malloc(sizeof(*sndf));
  memset(sndf, 0, sizeof(*sndf));
  sndf->f = wave_f;

  sndf->bigendian = bigendian;
  sndf->channels = channels;
  sndf->samplebytes = samplebytes;
  sndf->samplerate = samplerate;
  //sndf->isfloat = 0;

  //计算采样点个数
  fseek(sndf->f, 0, SEEK_END);
  sndf->samples = ftell(sndf->f) / (sndf->channels * sndf->samplebytes);
  rewind(sndf->f);

  return sndf;
}

size_t pcm_read_float32(pcmfile_t *sndf, float *buf, size_t num)
{
  size_t cnt;
  size_t isize;
  void *bufi;

  isize = num * sndf->samplebytes;        //期望读取的数据长度   num=2048  isize=4096  buf=0x631aa0
  bufi = (buf + num);                     //移动到期望的内存结尾  bufi = buf   bufi=0x633aa0   向前移动了8192
  bufi -= isize;                          //又移动回来了，这是什么操作哦???     bufi=0x632aa0   向后移动了4096
  isize = fread(bufi, 1, isize, sndf->f); //isize变成了实际读取到的数据大小
  isize /= sndf->samplebytes;             //isize变成了实际读取到的采样数量

  // perform in-place conversion
  for (cnt = 0; cnt < num; cnt++)
  {
    if (cnt >= isize) //isize 为实际返回的采样数量
      break;

    switch (sndf->samplebytes)
    {
    case 1:
    {
      uint8_t *in = bufi;
      uint8_t s = in[cnt];

      buf[cnt] = ((float)s - 128.0) * (float)256;
    }
    break;
    case 2:
    {
      int16_t *in = bufi;
      int16_t s = in[cnt];
#ifdef WORDS_BIGENDIAN
      if (!sndf->bigendian)
#else
      if (sndf->bigendian)
#endif
        buf[cnt] = (float)SWAP16(s);
      else
        buf[cnt] = (float)s;
    }
    break;
    case 3:
    {
      int s;
      uint8_t *in = bufi;
      in += 3 * cnt;

      if (!sndf->bigendian)
        s = in[0] | (in[1] << 8) | (in[2] << 16);
      else
        s = (in[0] << 16) | (in[1] << 8) | in[2];

      // fix sign
      if (s & 0x800000)
        s |= 0xff000000;

      buf[cnt] = (float)s / 256;
    }
    break;
    case 4:
    {
      int32_t *in = bufi;
      int s = in[cnt];
#ifdef WORDS_BIGENDIAN
      if (!sndf->bigendian)
#else
      if (sndf->bigendian)
#endif
        buf[cnt] = (float)SWAP32(s) / 65536;
      else
        buf[cnt] = (float)s / 65536;
    }
    break;
    default:
      return 0;
    }
  }

  return cnt;
}

int pcm_close(pcmfile_t *sndf)
{
  int i = fclose(sndf->f);
  free(sndf);
  return i;
}
