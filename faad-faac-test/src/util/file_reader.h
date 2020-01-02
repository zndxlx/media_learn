#ifndef FILE_READER_H_INCLUDED
#define FILE_READER_H_INCLUDED

typedef struct
{
    long bytes_into_buffer; //buffer中的数据量
    long bytes_consumed;    //记录在buffer中已经消费的bytes大小
    //long file_offset;       //记录文件有效数据的偏移
    unsigned char *buffer; //读取文件的buffer
    int buffer_size;       //读取文件的buffer大小
    int at_eof;            //是否读取到文件尾部
    FILE *infile;          //文件指针
    const char *path;      //文件路径
} file_reader;

file_reader *file_reader_create(const char *path, int buffer_size);

void file_reader_destroy(file_reader *reader);

/*填充buffer,消费数据后调用，将文件中的数据读取填充到buffer中*/
int file_reader_fill_buffer(file_reader *reader);

/*消费数据 bytes 消费大小，不能超过bytes_into_buffer*/
int file_reader_advance_buffer(file_reader *reader, int bytes);

#endif