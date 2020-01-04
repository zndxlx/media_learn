rfc3640
https://www.cnblogs.com/djw316/p/10975372.html


测试
./test ./demo.aac 192.168.50.197 6664

ffplay -protocol_whitelist "file,rtp,udp" a.sdp


怎么打包可以参考ffmpeg rtpenc_aac.c 


1、需要去掉adts头，

2、需要增加au头
   au头好像一般设置为4个字节， 前面两个字节表示是au的长度
   后面的

在sdp中 sizeLength表示了长度字段在AU-size()中怎么取，indexLength表示了AU-Index怎么取，
假如
a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=119056E500

packet[0] = 0x00;  
packet[1] = 0x10;  
packet[2] = (len & 0x1fe0) >> 5;  
packet[3] = (len & 0x1f) << 3; 

packet[0] packet[1]，表示AU-header的长度为16，sizelength=13表示 au-header中前面13个bit表示AU-size，
所以 packet[2]  packet[3] 中才能表示这个au的长度

在ffmpeg rtpenc_aac.c会一次发送多个aac包，packet[1]可能为0x30(3个包)，0x40(4个包); 后面对应AU-header1

比如一个包长度为


      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .. -+-+-+-+-+-+-+-+-+-+
      |AU-headers-length|AU-header|AU-header|      |AU-header|padding|
      |                 |   (1)   |   (2)   |      |   (n)   | bits  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .. -+-+-+-+-+-+-+-+-+-+

                   Figure 2: The AU Header Section


au 头
AU-header
      +---------------------------------------+
      |     AU-size                           |
      +---------------------------------------+
      |     AU-Index / AU-Index-delta         |
      +---------------------------------------+
      |     CTS-flag                          |
      +---------------------------------------+
      |     CTS-delta                         |
      +---------------------------------------+
      |     DTS-flag                          |
      +---------------------------------------+
      |     DTS-delta                         |
      +---------------------------------------+
      |     RAP-flag                          |
      +---------------------------------------+
      |     Stream-state                      |
      +---------------------------------------+
      
      
 一些记录
   1、一个AAC原始帧包含一段时间内1024个采样及相关数据
