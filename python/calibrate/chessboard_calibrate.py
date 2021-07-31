"""
基于提取棋盘角点定位的显示器标定算法
"""
import cv2
import sys
import numpy as np
import argparse

class ChessBoardCalibrater:
    def __init__(self, screen_size=(1080, 1920), is_test=False):
        """
        :param screen_size:显示器的分辨率（宽*高）
        :param is_test:开启测试功能
        """
        self.screen_size = screen_size
        self.is_test = is_test
        self.chessboard_size = (7, 5)  # 棋盘的角点个数(列 * 行)
        self.chessboard_points = None
        self.H = None

    def reflect_to_screen_coords(self, coords):
        """
        将摄像头检测到的目标坐标映射成显示器的坐标
        :param coords:要映射的坐标，格式为[x, y]
        """
        coords = np.array([[coords[0]], [coords[1]], [1]], np.float)
        coords = self.H[0].dot(coords)
        coords //= coords[2][0]
        
        return [coords[0][0], coords[1][0]]
        
    def generatePattern(self):
        """
        生成大小为屏幕大小，棋盘内角点个数为self.chessboard_size的棋盘图片
        """
        CheckerboardSize = min(self.screen_size) // 10
        Nx_cor, Ny_cor = self.chessboard_size

        black = np.zeros((CheckerboardSize, CheckerboardSize, 3), np.uint8)
        white = np.zeros((CheckerboardSize, CheckerboardSize, 3), np.uint8)
        black[:] = [0, 0, 0]  # 纯黑色
        white[:] = [255, 255, 255]  # 纯白色
     
        black_white = np.concatenate([black, white], axis=1)
        black_white2 = black_white
        white_black = np.concatenate([white, black], axis=1)
        white_black2 = white_black
     
        # 横向连接
        if Nx_cor % 2 == 1:
            for i in range(1, (Nx_cor+1) // 2):
                black_white2 = np.concatenate([black_white2, black_white], axis=1)
                white_black2 = np.concatenate([white_black2, white_black], axis=1)
        else:
            for i in range(1, Nx_cor // 2):
                black_white2 = np.concatenate([black_white2, black_white], axis=1)
                white_black2 = np.concatenate([white_black2, white_black], axis=1)
            black_white2 = np.concatenate([black_white2, black], axis=1)
            white_black2 = np.concatenate([white_black2, white], axis=1)
     
        jj = 0
        black_white3 = black_white2
        for i in range(0, Ny_cor):
            jj += 1
            # 纵向连接
            if jj % 2 == 1:
                black_white3 = np.concatenate((black_white3, white_black2))  # =np.vstack((img1, img2))
            else:
                black_white3 = np.concatenate((black_white3, black_white2))  # =np.vstack((img1, img2))

        # 通过在四周padding将图片拼接为全屏幕大小
        curr_shape = black_white3.shape
        pad_h = self.screen_size[0] - curr_shape[0]
        pad_w = self.screen_size[1] - curr_shape[1]

        pad_top_bottom = np.zeros((pad_h // 2, curr_shape[1], 3), np.uint8)
        pad_top_bottom [:] = [255, 255, 255]
        result = np.concatenate([pad_top_bottom, black_white3], axis=0)
        result = np.concatenate([result, pad_top_bottom], axis=0)

        pad_left_right = np.zeros((self.screen_size[0], pad_w // 2, 3), np.uint8)
        pad_left_right[:] = [255, 255, 255]
        result = np.concatenate([pad_left_right, result], axis=1)
        result = np.concatenate([result, pad_left_right], axis=1)

        # 保存棋盘点的坐标（这个检测不可能失败）
        success, self.chessboard_points = self.detect_chess_board(result)   
        if not success:
            print("self.detect_chess_board(result) failed")
            exit(0)
     
        # 全屏显示
        cv2.namedWindow("calibrate",cv2.WINDOW_NORMAL)
        cv2.setWindowProperty("calibrate", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
        cv2.imshow("calibrate", result)
        cv2.waitKey(100)

    def detect_chess_board(self, img):
        """
        寻找img中的棋盘角点
        :param img:要检测的图片
        """
        success, corners = cv2.findChessboardCorners(img, self.chessboard_size)
        return success, corners
    
    def calibrate(self, cap=0):
        """
        打开摄像头，进行标定
        :param cap:传入的相机流。当外部程序调用此函数时，应当传入外部程序使用的摄像头流
        """
        if type(cap) == cv2.VideoCapture:
            pass
        else:
            cap = cv2.VideoCapture(cap)
            cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
            cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

        # 设置摄像头曝光度，使其可以检测到显示器上显示的棋盘(不同摄像头类型有着不同的参数)
        save = cap.get(cv2.CAP_PROP_EXPOSURE)
        cap.set(cv2.CAP_PROP_EXPOSURE, 30)

        # 在显示器上显示棋盘图片，摄像头捕获并显示器图片进行标定
        self.generatePattern()

        # 开始标定
        cnt = 0
        while cap.isOpened():

            # 标定超时
            cnt += 1
            if not self.is_test and cnt > 50:
                break

            success, frame = cap.read()
            if not success:
                print("cap.read() failed, retrying")
                continue

            # 提取角点并计算单应矩阵
            success, corners = self.detect_chess_board(frame)
            if not success:
                print("not found corners, retrying")
                continue
            self.H = cv2.findHomography(corners.squeeze() ,self.chessboard_points.squeeze()) 

            # 在测试模式下显示标定效果
            if self.is_test:
                result = cv2.warpPerspective(frame, self.H[0], dsize=(self.screen_size[1], self.screen_size[0]))
                cv2.namedWindow("frame", cv2.WINDOW_NORMAL)
                cv2.namedWindow("result", cv2.WINDOW_NORMAL)
                cv2.imshow("frame", frame)
                cv2.imshow("result", result)
                cv2.waitKey(10)
            else:
                break

        # 处理结果
        if cnt > 100:
            print("Error: calibrate timeout!")
        else:
            print('calibrate success, get param\nH = {}'.format(self.H[0]))

        # 销毁窗口，恢复原先的摄像头参数
        cv2.destroyAllWindows()
        cap.set(cv2.CAP_PROP_EXPOSURE, save)

def main(opt):
    calibrater = ChessBoardCalibrater(screen_size=(opt.height, opt.width) ,is_test=opt.test)
    calibrater.calibrate(opt.source) 

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Use a checkerboard grid to calibrate the position of the camera in relation to the monitor")
    parser.add_argument("--source", type=int, default=0, help="file of videos, 0,1... for camera")
    parser.add_argument("--width", type=int, default=1920, help="screen width resolution")
    parser.add_argument("--height", type=int, default=1080, help="screen height resolution")
    parser.add_argument("--test", action="store_true",default=False, help="test mode")

    opt = parser.parse_args()
    main(opt)
