#include "stdio.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"

const char *inputFileName = "CITY_352x288_30_orig_01.yuv";   //输入文件名
const char *outputFileName = "a.h264";  //输出文件名
const int frameW = 352;   //输入帧宽度
const int frameH = 288;  //输入帧高度
const int bitRate = 250000;   //编码码率
const int frameToEncode = 100;  //共编码多少帧
const int gopSize = 12;   //gop大小

//从fin中读取数据到frame的data中，frame->data[0]保存Y, frame->data[1]保存U, frame->data[2]保存V
//color 0:Y, 1:U, 2:V
int read_yuv_data(int color, AVFrame *frame, FILE* fin) {
    int height = color == 0 ? frameH : frameH / 2;
    int width = color == 0 ? frameW : frameW / 2;
    int pix_size = height * width;
    
    int color_stride = frame->linesize[color];  //frame->linesize 中保存的YUV中一行的数据长度，可能为了对齐需要填0
    
    if (width == color_stride){
        fread(frame->data[color], pix_size, 1, fin);
    } else {
        for (int row = 0; row < height; row++){
           fread(frame->data[color] + row * color_stride, width, 1, fin); 
        }
    }
    return pix_size;
}

int main(void) {  
    int ret = 0;
    FILE* fin;
    FILE* fout;
    
    //打开输入输出文件
    if((fin = fopen(inputFileName, "rb"))==NULL) {
        printf("cant open the file %s\n", inputFileName);
        return -1;
    }
    
    if((fout = fopen(outputFileName, "wb"))==NULL) {
        printf("cant open the file %s\n", outputFileName);
        return -1;
    }
    
    
    //注册ffmpeg所有编解码器组件
    avcodec_register_all();

    //查找AVcodec编解码器
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (codec == NULL) {
        printf("avcode_find_encoder failed\n");
        return -1;
    }
    
    //分配AVCodecContext实例
    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    if (codecCtx == NULL) {
        printf("avcodec_alloc_context3 failed\n");
        return -1;
    }
    
    //设置编码器参数
    codecCtx->width = frameW;
    codecCtx->height = frameH;
    codecCtx->bit_rate = bitRate;
    codecCtx->gop_size = gopSize;
    
    codecCtx->time_base = (AVRational){1, 30};    //设置帧率  1秒30帧
    codecCtx->max_b_frames = 1;     //一个gop中最大b帧数。 如果是视频会议，直播等，是不应该有b帧的，设置为0   
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    av_opt_set(codecCtx->priv_data, "preset", "slow", 0);  //自定义数据  ... 还不明白为什么这么设置
    
    //打开编码器
    ret =  avcodec_open2(codecCtx, codec, NULL); 
    if (ret != 0) {
        printf("avcodec_open2 failed, ret = %d\n", ret);
        return -1;
    }
    
    //分配AVFrame像素存储空间
    AVFrame *frame = av_frame_alloc();
    if (frame == NULL){
        printf("av_frame_alloc failed\n");
        return -1;
    }
    
    frame->width = codecCtx->width;
    frame->height = codecCtx->height;
    frame->format = codecCtx->pix_fmt;
    
    //frame->data 是申请的数据空间, 是一个数组分别保存Y,U,V的数据
    //frame->linesize 是申请的行大小,是一个数组分别表示Y,U,V
    ret = av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, frame->format, 32);
    if (ret <= 0) {
        printf("av_image_alloc failed ret=%d\n", ret);
        return -1;
        
    }
    
    AVPacket pkt;
    int got_packet = 0;
    for (int i = 0; i < frameToEncode; i++) {
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;

        read_yuv_data(0, frame, fin);
        read_yuv_data(1, frame, fin);
        read_yuv_data(2, frame, fin);
        frame->pts = i;
        
        got_packet = 0;
        ret = avcodec_encode_video2(codecCtx, &pkt, frame, &got_packet);
        if (ret != 0) {
            printf("avcodec_encode_video2 failed ret= %d\n", ret);
            return -1;
        }
        
        if (got_packet){
            printf("Write packet of frame %d, size=%d\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, fout);
            av_packet_unref(&pkt);  //会reset pkt.data, pkt.size
        }
    }
    
    
    //将缓冲中的数据输出
    got_packet = 1;
    while (got_packet) {
        ret = avcodec_encode_video2(codecCtx, &pkt, NULL, &got_packet);
        if (ret != 0) {
            printf("avcodec_encode_video2 failed ret= %d\n", ret);
            return -1;
        }
        
        if (got_packet) {
            printf("Write cache packet size=%d\n", pkt.size);
            fwrite(pkt.data, 1, pkt.size, fout);
            av_packet_unref(&pkt);
        }
    }
    
    fclose(fin);
    fclose(fout);
    
    avcodec_close(codecCtx);
    av_free(codecCtx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    

    return 0;
}