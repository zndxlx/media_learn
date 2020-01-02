ortp安装
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


