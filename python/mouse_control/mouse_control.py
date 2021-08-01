"""
调用标定模块、检测模块、追踪模块，调用gui模块控制鼠标
"""
import os
import sys
sys.path.append(os.getcwd())

import cv2
from calibrate.chessboard_calibrate import ChessBoardCalibrater
from detect.threshold_detector import Detector
from track.basic_tracker import Tracker
from timeit import default_timer as timer
import argparse
import pyautogui

class mouseControl:
    def __init__(self, source=0, screen_type=0, screen_resolution=(1080, 1920), camera_resolution=(1080, 1920)):
        """
        :param source: 摄像头的输入源
        :param screen_type: 屏幕类型 0：投影仪 1：LCD显示器
        :param screen_resolution: 屏幕分辨率
        :param camera_resolution: 摄像头分辨率
        """
        self.cap = cv2.VideoCapture(source)
        self.calibrater = ChessBoardCalibrater(screen_resolution)
        self.detector = Detector(source, screen_type, camera_resolution)
        self.tracker = Tracker(10)

        # 设置pyautogui
        pyautogui.FAILSAFE = False
        pyautogui.PAUSE = 0.001
        
    def run(self):
        self.detector.set_camera(self.cap)  # 检测前设置相机
        self.calibrater.calibrate(self.cap) # 标定

        while self.cap.isOpened():
            success, frame = self.cap.read()
            if not success:
                print("camera read failed, exited")
                exit(0)

            # 检测并追踪
            t1 = timer()
            coords = self.detector.detect(frame)
            t2 = timer()
            print("cost time {} ms".format((t2 - t1) * 1000))
            self.tracker.update(coords) 
            coords = self.tracker.position()

            # 检测到目标时，控制鼠标
            if len(coords):
                screen_coords = self.calibrater.reflect_to_screen_coords(coords) 
                pyautogui.moveTo(screen_coords[0][0], screen_coords[0][1])
                #frame = cv2.circle(frame, (coords[0][0], coords[0][1]), radius=5, color=(0, 0, 255))

def main(opt):
    controller = mouseControl(source=opt.source, screen_type=opt.screen_type, 
            screen_resolution=(opt.screen_height, opt.screen_width), camera_resolution=(opt.camera_height, opt.camera_width))
    controller.run()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="detect laser point position and control mouse action")
    parser.add_argument("--source", type=int, default=0, help="input source, 0,1... for camera")
    parser.add_argument("--screen_type", type=int, default=0, help="screen type, 0 for projector, 1 for LCD" )
    parser.add_argument("--screen_height", type=int, default=1080, help="screen height resolution" )
    parser.add_argument("--screen_width", type=int, default=1920, help="screen width resolution" )
    parser.add_argument("--camera_height", type=int, default=480, help="camera width resolution" )
    parser.add_argument("--camera_width", type=int, default=640, help="camera width resolution" )
    opt = parser.parse_args()

    main(opt)
