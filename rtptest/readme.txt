一、ortp安装
https://blog.csdn.net/wang3141128/article/details/80481111?utm_source=blogxgwz1
1、cmake3安装
https://www.cnblogs.com/chenjunwu/p/11730460.html
2、mbedtls库安装
cmake . -DUSE_SHARED_MBEDTLS_LIBRARY=On
3、bctoolbox库安装
https://github.com/BelledonneCommunications/bctoolbox  下载0.6.0版本
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DENABLE_TESTS_COMPONENT=NO
4、ortp 1.0.2版本安装
cmake . -DCMAKE_INSTALL_PREFIX=/usr

需要将bctoolbox的库和ortp的库拷贝出来。



测试

转码一个g711a音频 注意要指定采样为和声道
ffmpeg.exe -i demo.aac -f alaw -ar 8000 -ac 1 demo.alaw
播放一下，需要指定格式和采样
ffplay.exe  demo.alaw -f alaw -ar 8000


发送rtp码流
./test demo.alaw 192.168.50.197 6664

播放rtp码流
ffplay.exe  -protocol_whitelist "file,rtp,udp" ./a.sdp


二、 mediastream安装
1、下载版本 mediastreamer-2.16.1
2、yum install libv4l  
   yum install libv4l-devel
   yum install libX11-devel
   参考 https://www.linuxidc.com/Linux/2018-10/154934.htm 安装ffmpeg和ffmpeg-libs
3、cmake3  -DENABLE_SPEEX_CODEC=NO -DENABLE_SPEEX_DSP=NO -DENABLE_FFMPEG=NO -DENABLE_V4L=0 -DENABLE_SOUND=NO -DENABLE_DEBUG_LOGS=YES -DENABLE_UNIT_TESTS=NO . 

cmake3  -DENABLE_SPEEX_CODEC=NO -DENABLE_SPEEX_DSP=NO  -DENABLE_SOUND=NO -DENABLE_DEBUG_LOGS=YES -DENABLE_UNIT_TESTS=NO . -DENABLE_V4L=0

cmake3  -DENABLE_SPEEX_CODEC=NO -DENABLE_SPEEX_DSP=NO -DENABLE_SOUND=NO -DENABLE_DEBUG_LOGS=YES -DENABLE_UNIT_TESTS=NO . 

cmake3  -DENABLE_SPEEX_CODEC=NO -DENABLE_SPEEX_DSP=NO -DENABLE_SOUND=NO -DENABLE_DEBUG_LOGS=YES -DENABLE_UNIT_TESTS=NO . 

make 
提示错误 
注释掉提示行 mediastreamer-2.16.1/src/base/msfactory.c
make install







