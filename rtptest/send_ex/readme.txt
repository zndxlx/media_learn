发送rtp码流
./test demo.alaw 192.168.50.197 6664

播放rtp码流
ffplay.exe  -protocol_whitelist "file,rtp,udp" ./a.sdp





转码得到一个g711a音频 注意要指定采样为和声道
ffmpeg.exe -i demo.aac -f alaw -ar 8000 -ac 1 demo.alaw
播放一下，需要指定格式和采样
ffplay.exe  demo.alaw -f alaw -ar 8000