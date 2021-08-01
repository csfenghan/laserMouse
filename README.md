# 基于激光笔的显示器交互

[项目原地址](https://github.com/csfenghan/laser_mouse)

本项目实现了在大屏投影仪下使用激光笔代替鼠标，充当无线鼠标的功能。显示器定位算法采用棋盘标定，激光点的检测算法通过调节摄像头的曝光时间来实现。

## 测试环境
- Ubuntu20.04    
- 当贝F3投影仪
- 摄像头使用的普通480\*640,USB摄像头
- 一个基于STM32的自制遥控器

## 硬件准备
- 一个可以软件设置曝光时间的USB摄像头
- 一个亮度较高的红色激光笔(也可以自己制作遥控器，本项目的遥控器通过串口进行通讯)

## 软件准备
- Linux任意发行版
- OpenCV4.0以上版本

## 使用  
首先下载源代码    

    $ git clone git@github.com:csfenghan/laserMouse.git    
    $ cd laserMouse

然后将接入摄像头，使摄像头可以拍摄到投影仪画面（显示器）的全貌，曝光清晰
实现分为C++和Python两个版本，Python为实验版本，C++为最终版本

### C++
C++版本使用OpenCV实现，进入目录后编译完成后运行即可

    $ cd cpp
    $ mkdir build && cd build
    $ cmake ..
    $ make
    $ ./laserMouse
运行后，使用激光笔照射屏幕，鼠标可以准确的跟随激光笔移动

### Python
Python版本的功能并不稳定，主要是用来测试一些功能

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
