#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "file_reader.h"

file_reader *file_reader_create(const char *path, int buffer_size)
{
    printf("file_reader_create enter\n");
    file_reader *reader = malloc(sizeof(file_reader));
    memset(reader, 0, sizeof(file_reader));

    //打开文件,
    reader->infile = fopen(path, "rb");
    if (reader->infile == NULL)
    {
        /* unable to open file */
        printf("Error opening file: %s\n", path);
        return NULL;
    }
    reader->path = path;
    reader->buffer_size = buffer_size;
    reader->buffer = malloc(buffer_size);
    memset(reader->buffer, 0, buffer_size);

    //读取文件填充到buffer中
    int bread = fread(reader->buffer, 1, buffer_size, reader->infile);
    reader->bytes_into_buffer = bread;
    reader->bytes_consumed = 0;
    //reader->file_offset = 0;
    if (bread != buffer_size)
        reader->at_eof = 1;
    return reader;
}

int file_reader_fill_buffer(file_reader *reader)
{
    if (reader->bytes_consumed > 0) //buffer已经被消费过了 才需要去填充
    {
        if (reader->bytes_into_buffer) //如果buffer中有数据，需要移动到头位置，将已经消费的数据移除
        {
            memmove((void *)reader->buffer, (void *)(reader->buffer + reader->bytes_consumed),
                    reader->bytes_into_buffer);
        }

        if (!reader->at_eof) //文件没有结束才能在文件中读取
        {
            //读取bytes_consumed大小内存填充到buffer尾部
            int bread = fread((void *)(reader->buffer + reader->bytes_into_buffer), 1,
                              reader->bytes_consumed, reader->infile);

            //读取后判断是否已经到达文件尾部
            if (bread != reader->bytes_consumed)
                reader->at_eof = 1;

            reader->bytes_into_buffer += bread;
        }

        reader->bytes_consumed = 0;
    }
    //读取
    return 0;
}

int file_reader_advance_buffer(file_reader *b, int bytes)
{
    if (bytes > b->bytes_into_buffer)
    {
        printf("error advace buffer, bytes > b->bytes_into_buffer");
        return -1;
    }
    b->bytes_consumed = bytes;
    b->bytes_into_buffer -= bytes;

    return 0;
}

void file_reader_destroy(file_reader *reader)
{
    printf("file_reader_destroy enter\n");
    //关闭文件
    fclose(reader->infile);
    //释放buffer内存
    free(reader->buffer);
    //释放自己
    free(reader);
    reader = NULL;
    return;
}