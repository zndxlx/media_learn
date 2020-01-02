在https://ffmpeg.zeranoe.com/builds/  下载win64的 ffmpeg-4.2.1-win64-dev 和ffmpeg-4.2.1-win64-shared 放在根目录下


一、说明
  1、示例为windows下编译需要需要使用mingw，参考三中下载mingw

二、配置环境ffmpeg

  1、下载ffmpeg https://ffmpeg.zeranoe.com/builds/ 下载dev和share,并解压缩
  2、将动态库添加到path环境变量下
  比如E:\ffproj\ffmpeg-4.2.1-win64-shared\bin
  3、打开工程make编译

三、mingw c环境安装

  1、安装mingw64
     https://blog.csdn.net/duke56/article/details/100187199
  2、修改配置文件
     https://code.visualstudio.com/docs/cpp/config-mingw
  3、makefile注意
     可能编译时候出现文件不存在
     删除不要用rm，用del代替