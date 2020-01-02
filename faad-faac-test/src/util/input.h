/*
 * FAAC - Freeware Advanced Audio Coder
 * Copyright (C) 2002 Krzysztof Nikiel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: input.h,v 1.7 2008/11/24 22:00:11 menno Exp $
 */

#ifndef _INPUT_H
#define _INPUT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

typedef struct
{
  FILE *f;
  int channels;
  int samplebytes; //一个采样点bytes数
  int samplerate;  //采样率
  int samples;     //采样点数
  int bigendian;   //是否是大端
  // int isfloat;
} pcmfile_t;

pcmfile_t *pcm_open_read(const char *name, int channels, int samplebytes, int bigendian, int samplerate);
//num 读取的采样点个数，返回值为实际读取的采样点个数
size_t pcm_read_float32(pcmfile_t *sndf, float *buf, size_t num);
int pcm_close(pcmfile_t *file);

#ifdef __cplusplus
}
#endif
#endif /* _INPUT_H */
