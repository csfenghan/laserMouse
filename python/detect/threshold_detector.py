"""
利用相机的曝光度来检测激光点在图片中的位置
"""

import cv2
import numpy as np
import argparse

class Detector:
    """
    负责检测图片中激光点的位置坐标
    """
    def __init__(self, screen_type=0, screen_resolution=(1080, 1920), camera_resolution=(1080, 1920)):
        """
        :param source: 输入源
        :param screen_type: 显示器的类型。0：投影仪 1：液晶显示器
        :param screen_resolution: 显示器分辨率
        :param camera_resolution: 摄像头分辨率
        """
        self.screen_type = screen_type
        self.screen_resolution = screen_resolution
        self.camera_resolution = camera_resolution
        self.save = [0, 0]  # 保存相机的CAP_PROP_AUTO_EXPOSURE和CAP_PROP_EXPOSURE参数

    def set_camera(self, cap):
        """
        根据显示器类型设置相机的参数(自动曝光、曝光时间)
        """
        # 设置数据格式（默认的YUV格式无法高帧率读取1080p数据）
        cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter.fourcc('M', 'J', 'P', 'G'))
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.camera_resolution[0])
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.camera_resolution[1])
        cap.set(cv2.CAP_PROP_FPS, 30)

        self.save[0] = cap.get(cv2.CAP_PROP_AUTO_EXPOSURE)
        self.save[1] = cap.get(cv2.CAP_PROP_EXPOSURE)

        # 关闭自动曝光
        cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1)
        if self.screen_type == 0:
            cap.set(cv2.CAP_PROP_EXPOSURE, 0)
        elif self.screen_type == 1:
            cap.set(cv2.CAP_PROP_EXPOSURE, 30)

    def detect(self, img, conf=0.5):
        """
        检测激光点位置。检测的原理是低曝光环境下只有激光点的值最高，通过二值化后进行采样可以进行检测
        :param img: 输入的图片
        :param conf: 筛选目标用的参数，conf越高，筛选的越准确，但有可能漏检；conf越低，检测到的目标越多，但可能错检
        :return coords: 检测到的激光点的坐标
        """
        # 灰度化然后二值化
        thresh = int(conf * 255)
        img = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
        ret, img= cv2.threshold(img, thresh, 255, cv2.THRESH_BINARY)

        # 寻找白色点的坐标
        img = np.array(img)
        white_points = np.where(img == 255)

        coords = []
        if len(white_points[0]):
            coords.append([int(np.mean(white_points[1])), int(np.mean(white_points[0]))])
        return coords

    def test(self, source):
        """
        测试模式
        """
        cap = cv2.VideoCapture(source)
        # 设置摄像头曝光
        self.set_camera(cap)

        while cap.isOpened():
            success, frame = cap.read()
            if not success:
                print("camera read failed, exited")
                exit(0)
            
            # 检测
            coords = self.detect(frame)
            if len(coords):
                cv2.circle(frame, center=(coords[0][0], coords[0][1]), radius=2, color=(0, 0, 255), thickness=-1)

            # 绘制结果
            cv2.namedWindow("frame", cv2.WINDOW_NORMAL)
            cv2.imshow("frame", frame)
            cv2.waitKey(10)

def main(opt):
    detector = Detector(screen_type=opt.screen_type)
    detector.test(opt.source)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="detect the laser point")
    parser.add_argument("--source", type=int, default=0, help="file of videos, 0,1... for camera")
    parser.add_argument("--screen_type", type=int, default=0, help="0 for projector, 1 for LCD or LED display")
    opt = parser.parse_args() 
    main(opt)
