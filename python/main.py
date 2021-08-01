import sys
import argparse
import threading
from mouse_control.mouse_control import MouseControl

# 接收命令，控制鼠标行为
class LaserMouse:
    def __init__(self)
        
        self.cmd_act = {
                "C" : {"fun" : self.calibrate_cmd, "help" : "calibrate the camera"},
                "P" : {"fun" : self.open_laser_cmd, "help": "start move mouse"},
                "p" : {"fun" : self.close_laser_cmd, "help" : "stop move mouse"},
                "L" : {"fun" : self.press_left_cmd, "help" : "click left key"},
                "l" : {"fun" : self.release_left_cmd, "help" : "release left key"},
                "R" : {"fun" : self.press_right_cmd, "help" : "click right key"},
                "r" : {"fun" : self.release_right_cmd, "help" : "release right key"}
                }

    # 从串口读取命令
    def getCmd(self):
        c = input()
        return c

    def help(self):
        print("\n")
        for key in self.cmd_act:
            print("{} : {}".format(key, self.cmd_act[key]["help"]))

    # 标定
    def calibrate_cmd(self):
        print("calibrating")
        self.mouse.calibrate()
    
    # 开启检测(即释放锁)
    def open_laser_cmd(self):
        print("start move mouse")
        self.lock.release()

    # 关闭检测(即获取锁)
    def close_laser_cmd(self):
        print("stop move mouse")
        self.lock.acquire()

    # 按下左键
    def press_left_cmd(self):
        print("click left key")

    # 松开左键 
    def release_left_cmd(self):
        print("release left key")

    # 按下右键
    def press_right_cmd(self):
        print("click right key")

    # 松开右键
    def release_right_cmd(self):
        print("release right key")

    # 主循环
    def run(self):
        self.mouse.start()  # 开启检测并控制鼠标线程
        while True:
            self.help()
            # 根据命令的值，执行对应的函数
            cmd = self.getCmd()
            if cmd in self.cmd_act:
                self.cmd_act[cmd]["fun"]()

def main():
    argv = sys.argv
    source = 0 if len(argv) == 1 else argv[1]

    lasermouse = LaserMouse(source, onnx_path="yolov5s.onnx", 
            inference_shape=(640, 640), camera_shape=(1080, 1920))
    lasermouse.run()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="")
    parser.add_argument("--source", type=int, default=0, "file of videos, 0,1... for camera")

    main()
