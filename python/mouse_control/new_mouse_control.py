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
        self.calibrater = ChessBoardCalibrater(screen_resolution=screen_resolution)
        self.detector = Detector(source, screen_type=screen_type, screen_resolution=screen_resolution, 
                camera_resolution=camera_resolution)
        self.tracker = Tracker(10)

        # 设置pyautogui
        pyautogui.FAILSAFE = False
        pyautogui.PAUSE = 0.001
        
    def run(self):
        self.calibrater.calibrate(self.cap) # 标定
        self.detector.set_camera(self.cap)  # 检测前设置相机

        # 摄像头刚刚打开时读取会有些问题，因此等待延迟一段时间
        for i in range(10):
            success, frame = self.cap.read()

        cnt = 0
        while self.cap.isOpened():
            success, frame = self.cap.read()

            # 检测并追踪
            img, coords = self.detector.detect(frame)
            self.tracker.update(coords) 
            coords = self.tracker.position()
            print(coords)

            # 检测到目标时，控制鼠标
            if len(coords):
                screen_coords = self.calibrater.reflect_to_screen_coords(coords) 
                pyautogui.moveTo(coords[0][0], coords[0][1])
                frame = cv2.circle(frame, (coords[0][0], coords[0][1]), radius=5, color=(0, 0, 255))
            #cv2.imshow("frame", frame)
            #cv2.imshow("img", img)
            #cv2.waitKey(30)

def main():
    controller = mouseControl(camera_resolution=(480,640))
    controller.run()

if __name__ == "__main__":
    main()
