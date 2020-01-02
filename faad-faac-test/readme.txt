一、faad2安装
参考 http://linuxfromscratch.org/blfs/view/svn/multimedia/faad2.html
1、下载
wget https://downloads.sourceforge.net/faac/faad2-2.8.8.tar.gz

2、编译
tar zxvf faad2-2.8.8.tar.gz
./configure --prefix=/usr
make
make DESTDIR=/home/lixin/aac/faad2-2.8.8/aaa install

3、将aaa目录下的lib bin include 目录拷贝出来


4、测试
下载测试文件 wget http://www.nch.com.au/acm/sample.aac

执行命令行工具
./bin/faad -o sample.wav sample.aac    
提示找不到库文件
export LD_LIBRARY_PATH=/home/lixin/aac/lib:$LD_LIBRARY_PATH
再执行




二、faac安装
参考 http://linuxfromscratch.org/blfs/view/svn/multimedia/faac.html
1、下载
wget https://downloads.sourceforge.net/faac/faac-1.29.9.2.tar.gz
2、编译
./configure --prefix=/usr
make
make DESTDIR=/home/lixin/aac/faac-1.29.9.2/aaa install
3、将aaa目录下的lib bin include 目录拷贝出来

4、测试
export LD_LIBRARY_PATH=/home/lixin/aac/lib:$LD_LIBRARY_PATH
./faac ../sample.wav -r -o a.mp4    #wav里面保存的是pcm数据,  
./faac ../sample.wav -r -o a.aac   #如果输出设置为.aac的，播放不了,因为指定了-r参数，输出为纯码流，少了adts头
将命令修改为
./faac ../sample.wav -o a.aac
可以正常播放了。
