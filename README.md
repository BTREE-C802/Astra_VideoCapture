# Astra_VideoCapture
启动方式：./astra_test ../File2/
第一项地址为在build下面的启动文件
第二项地址为数据需要储存的地址（"File2"可自由更改，但是必须以“/”结束）,../File2/为build上级目录下新建一个名为File1的文件夹，数据集则采集于这一文件夹下。如果采集数据正确，该文件夹可正常创建，而且下面会有新的文件和文件夹产生，分别为depth和rgb文件夹，这两个文件夹储存拍摄的深度图片可RGB图片，depth.txt和rgb.txt为对应图像命名与时间戳。

使用步骤：
第一步：在Astra_test目录下新建一个名为build的文件夹
第二步：cd build
第三步：cmake ..
第四步：make 
第五步：插入Astra s 型号摄像头，然后在终端中输入启动方式

该算法为使用Astra-s摄像头采集3DLine-SLAM所使用的数据集，该数据集的形式和TUM  RGB-D 数据集的形式是一样的，采集的数据集可以在ORB-SLAM程序上进行使用，需要说明的是：3DLine-SLAM算法为我们新开发的算法，该算法在ORB-SLAM上进行开发。

注意Astra s的驱动器安装必须自行先安装测试良好后才可使用我们的软件采集数据集，安装方式参考如下：
Linux系统  Ubuntu14.04 indigo版本
安装依赖：
sudo apt-get install build-essential freeglut3 freeglut3-dev

检查udev版本，需要libudev.so.1，如果没有则添加
$ldconfig -p | grep libudev.so.1
$cd /lib/x86_64-linux-gnu
$sudo ln -s libudev.so.x.x.x libudev.so.1

下载驱动：https://orbbec3d.com/develop/#registergestoos，选择linux版本
(下载这个：Download Orbbec OpenNI SDK   下载后选择Linux   里面就有OpenNI-Linux-x64-2.3 {根据自己版本选择})

$ cd ~
$ wget http://www.orbbec3d.net/Tools_SDK_OpenNI/2-Linux.zip

选择OpenNI-Linux-x64-2.3解压

$ unzip OpenNI-Linux-x64-2.3.zip
$ cd OpenNI-Linux-x64-2.3

安装
$ sudo chmod a+x install.sh
$ sudo ./install.sh

重插设备
加入环境
$ source OpenNIDevEnvironment

**也可以在github上下载**https://github.com/choonyip/OpenNI-Linux-x64-2.3中的内容，安装方法稍有差异

编译例子
$ cd Samples/SimpleViewer
$ make
连接设备，执行例子
$ cd Bin/x64-Release
$ ./SimpleViewer
如果一切正常没有问题，则能显示图像！
