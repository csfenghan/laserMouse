"""
调用摄像头，检测激光点在屏幕中的位置，并控制鼠标执行相应的行为
"""
import os
import sys
sys.path.append(os.getcwd())

import cv2
import threading
import pyautogui as pg
import argparse
from timeit import default_timer as timer
from calibrate.chessboard_calibrate import ChessBoardCalibrater
from detect.threshold_detector import Detector
from track.basic_tracker import Tracker

class MouseControl(threading.Thread):
    def __init__(self, lock=threading.Lock(), source=0, onnx_path="yolov5s.onnx", 
           inference_shape=(640, 640), camera_shape=(1080, 1920), name="MouseControl", is_test=False):
        """
        :param lock: 一个互斥锁，只有self持有锁时才可以进行推理，否则休眠
        :param source: 视频的输入源
        :param onnx_path: onnx文件的路径
        :param inference_shape: onnx文件的推理输入大小
        :param camera_shape: 摄像头的分辨率
        :param name: 线程名
        :param is_test: 测试模式
        """
 
        super().__init__(name=name)

        # 设置摄像头分辨率
        self.cap = cv2.VideoCapture(source)         
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, camera_shape[1])
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, camera_shape[0])
        self.cap.set(cv2.CAP_PROP_FPS, 30)
        if not self.cap.isOpened():
            print("open camera failed")
            exit(0)

        # 查看设置
        width = self.cap.get(cv2.CAP_PROP_FRAME_WIDTH)
        height = self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
        fps = self.cap.get(cv2.CAP_PROP_FPS)
        print("camera input size: height: {}, width: {}, fps: {}".format(height, width, fps))

        self.detector = Detector(source=source, screen_type=0)
        self.lock = lock    # 使用lock来实现检测开关的控制
        self.calibrater = ChessBoardCalibrater(screen_size=(1080, 1920))
        self.tracker = Tracker(delay_cnt=10)
        self.is_test = is_test
    
    def calibrate(self):
        """
        标定显示器位置
        """
        self.calibrater.calibrate(self.cap)

    def cal_coords(self, coords):
        """
        将摄像头拍摄到的坐标转换成在屏幕上的坐标
        :param coords: 摄像头检测到的坐标。格式 [x, y]
        """
        return self.calibrater.reflect_to_screen_coords(coords)
   
    def run(self):
        # 设置pyautogui的配置
        pg.FAILSAFE = False
        pg.PAUSE = 0.001

        # 运行时会更改camera的配置，因此提前保存
        save_auto = self.cap.get(cv2.CAP_PROP_AUTO_EXPOSURE)
        save_exposure = self.cap.get(cv2.CAP_PROP_EXPOSURE)

        self.cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1) # 设置为手动曝光
        self.cap.set(cv2.CAP_PROP_EXPOSURE, 1)  # 设置低曝光度

        # 相机标定
        self.calibrater.calibrate(self.cap) 

        cnt = 0
        while self.cap.isOpened():
            # 如果可以获得锁，则说明当前允许运行。否则阻塞
            self.lock.acquire()
            self.lock.release()

            # 检测图片中激光点的位置
            success,img = self.cap.read() 
            if not success:
                cnt += 1
                if cnt > 100:
                    print("cap.read() failed, exit")
                    break
                print("read camera failed, retrying~")
                continue

            # 检测
            time1 = timer()
            success, pos = self.detector.detect(img)
            self.tracker.update(pos)
            pos = self.tracker.position() 
            time2 = timer()
            print("infernece cost time: {}ms".format((time2 -time1) * 1000))

            # 如果检测到激光点，则使用其控制鼠标
            if len(pos):
                pos = pos[0]
                coords = self.calibrater.reflect_to_screen_coords(pos)
                pg.moveTo(coords[0], coords[1])

                # 测试模型下的功能
                if self.is_test:
                    img = cv2.circle(img, (coords[0], coords[1]), radius=3, color=(0, 0, 255), thickness=1)

            if self.is_test:
                cv2.namedWindow("frame", cv2.WINDOW_NORMAL)
                cv2.imshow("frame", img)
                cv2.imwrite("frame.png", img)
                cv2.waitKey(1)

        # 异常退出
        print("Thread {} exited".format(self.name))

def main(opt):
    mousecontrol = MouseControl(camera_shape=(opt.height, opt.width), is_test=opt.test, source=opt.source)
    mousecontrol.calibrate()
    mousecontrol.run()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test detection and mouse control functions") 
    parser.add_argument("--source", type=int, default=0, help="file of videos, 0,1... for camera")
    parser.add_argument("--width", type=int, default=640, help="camera width resolution")
    parser.add_argument("--height", type=int, default=480, help="camera height resolution")
    parser.add_argument("--test", action="store_true", default=False, help="test mode")

    opt = parser.parse_args()
    main(opt)
