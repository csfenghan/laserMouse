# 基于激光笔的显示器交互

[项目原地址](https://github.com/csfenghan/laser_mouse)

本项目实现了在大屏投影仪下使用激光笔代替鼠标，充当无线鼠标的功能。显示器定位算法采用棋盘标定，激光点的检测算法通过调节摄像头的曝光时间来实现。

## 测试环境
- Ubuntu20.04    
- 当贝F3投影仪
- 摄像头使用的普通480\*640,USB摄像头
- 一支红色激光笔

## 硬件准备
- 一个可以软件设置曝光时间的USB摄像头
- 一个任意颜色的激光笔  
ps:如果只是测试其控制鼠标的基本功能，则只需要一个激光笔即可。如果要自制遥控器，则只需要按照指令列表发送相应命令即可

## 软件准备
- Linux任意发行版
- OpenCV4.0以上版本
- jsoncpp库

## 使用  
安装jsoncpp库  

    $ sudo apt install libjsoncpp-dev

首先下载源代码    

    $ git clone https://github.com/csfenghan/laserMouse.git
    $ cd laserMouse

然后将接入摄像头，使摄像头可以拍摄到投影仪画面（显示器）的全貌，并且曝光清晰

实现分为C++和Python两个版本，Python为实验版本，C++为最终版本

### C++
C++版本实现了鼠标控制的全部功能，依赖OpenCV和X11实现，进入目录后编译完成后运行即可。（使用前需根据自己的硬件配置修改config.json文件中的摄像头和显示器的配置）

    $ cd cpp
    $ mkdir build && cd build
    $ cmake ..
    $ make
    $ ./laserMouse ../config.json
运行后，使用激光笔照射屏幕，鼠标可以准确的跟随激光笔移动。如果没有遥控器但想要模仿鼠标的行为，可以使用键盘模仿来自遥控器的信号（具体用法程序运行后有相应的help提示）

### Python
Python版本的功能并不稳定也不完善，主要是用来测试一些功能

    $ cd laser_mouse/python
    $ pip install -r requirements.txt
    $ python3 mouse_control/mouse_control.py 

## 其它
1. 查看显示器与摄像头的标定效果

    $ python3 chessboard_calibrate.py
    
 或者C++版本中Calibrater实现了test函数，也可以用来测试标定效果

2. 如果只是要测试激光点定位的效果，运行
    
    $ python3 mouse_control.py

 或者C++版本中Detector实现了test函数，也可以用来测试检测效果
