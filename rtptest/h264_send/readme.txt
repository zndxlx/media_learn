














一、注意点
1、h264 RTP 时间戳被设置为内容的采样时间戳。必须使用 90 kHz 始终频率，因此时间戳的单位为1(秒)/90000，
如果当前视频帧率为25fps，那时间戳间隔或者说增量应该为3600，如果帧率为30fps，则增量为3000
2、包类型分为 单个NAL单元包， 聚合包(有四种)， 分片单元(FU-A,FU-B)，FU-A常用

NAL Unit  Packet    Packet Type Name               Section
Type      Type
-------------------------------------------------------------
0        reserved                                     -
1-23     NAL unit  Single NAL unit packet             5.6
24       STAP-A    Single-time aggregation packet     5.7.1
25       STAP-B    Single-time aggregation packet     5.7.1
26       MTAP16    Multi-time aggregation packet      5.7.2
27       MTAP24    Multi-time aggregation packet      5.7.2
28       FU-A      Fragmentation unit                 5.8
29       FU-B      Fragmentation unit                 5.8

3、打包模式
 0 单 NAL 单元模式  1 非交错模式  2 交错模式
 一般都用1 

4、rtp timestamp 
如果分包了，每个分包都使用相同的时间戳。从裸流中怎么计算出来？？？
https://blog.51cto.com/70565912/533736?source=dra
   4.1、根据sps 计算帧率(测试文件为60帧每秒)
   sps = time_scale/num_units_in_tick

5、fu-a 包括 FU indicator， FU header。
FU indicator有以下格式：
      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |F|NRI|  Type   |
      +---------------+
 FU指示字节的类型域 Type=28表示FU-A。。NRI域的值必须根据分片NAL单元的NRI域的值设置。
FU header的格式如下：
      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |S|E|R|  Type   |
      +---------------+
   S: 1 bit
   当设置成1,开始位指示分片NAL单元的开始。当跟随的FU荷载不是分片NAL单元荷载的开始，开始位设为0。
   E: 1 bit
   当设置成1, 结束位指示分片NAL单元的结束，即, 荷载的最后字节也是分片NAL单元的最后一个字节。当跟随的FU荷载不是分片NAL单元的最后分片,结束位设置为0。
   R: 1 bit
   保留位必须设置为0，接收者必须忽略该位。
   Type: 5 bits
   NAL单元荷载类型和分片NAL单元nal unit type保持一致

Type   Packet      Type name                       
      ---------------------------------------------------------
      0      undefined                                    -
      1-23   NAL unit    Single NAL unit packet per H.264  
      24     STAP-A     Single-time aggregation packet    
      25     STAP-B     Single-time aggregation packet    
      26     MTAP16    Multi-time aggregation packet     
      27     MTAP24    Multi-time aggregation packet     
      28     FU-A      Fragmentation unit                
      29     FU-B      Fragmentation unit                 
      30-31  undefined 

6、如果不分包，直接把nal单元直接封装就好了

参考文档
https://www.wolfcstech.com/2017/08/18/h264_on_rtp/
https://www.cnblogs.com/likwo/p/3533392.html
RFC 6184
ffmpeg-4.0.2\libavformat\rtpenc_h264_hevc.c
