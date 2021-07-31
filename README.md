# 基于激光笔的显示器交互

[项目原地址](https://github.com/csfenghan/laser_mouse)

使用激光笔与显示器进行交互，代替鼠标的功能。需要的硬件有
- 一个可以软件设置曝光时间的高帧率摄像头
- 一个亮度较高的红色激光笔
- （可选）自制相应的控制遥控器

## 安装   
### Python
    $ git clone git@github.com:csfenghan/laser_mouse.git
    $ cd laser_mouse/python
    $ pip install -r requirements.txt

### C++
C++版本还在开发中

## 使用
1. 接入摄像头，保证摄像头可以拍摄到显示器的全貌
2. 下载模型参数    
    $ wget https://github.com/csfenghan/laser_mouse/releases/download/v0.1-alpha/weights.tar.gz && tar -xvf weights.tar.gz
2. 运行指令 `python3 main.py`，根据提示即可完成操作

ps:尽量选用高亮度的激光发射器，另外要保证摄像头不会过度曝光

## 其它
如果要查看显示器位置标定的效果，运行

    $ python3 chessboard_calibrate.py

如果只是要测试激光点定位的效果，运行
    
    $ python3 mouse_control.py
