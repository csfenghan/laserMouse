"""
已经废弃的方法，效果不是很好，误差较大，请使用chessboard_calibrate.py中的接口
使用二维码定位的方式来定位显示器
"""
import cv2
import math
import numpy as np

# 
class Calibrater:
    def __init__(self, height=1650, width=2200):
        self.screen_height = height
        self.screen_width = width
        self.img_height = 0
        self.img_width = 0
        self.base_len = min(height, width) // 28
        self.H = None
        self.ratio = None

    # 在指定位置显示 回 字，与QR二维码的格式相同
    def display_object(self, img, start, base_len):
        end = np.array([start[0] + 7 * base_len, start[1] + 7 * base_len]) 

        cv2.rectangle(img, start, end, (0, 0, 0), -1)

        start += base_len
        end -= base_len
        cv2.rectangle(img, start, end, (255, 255, 255), -1)
        
        start += base_len
        end -= base_len
        cv2.rectangle(img, start, end, (0, 0, 0), -1)

    # 显示图片
    def display(self):
        height = self.screen_height
        width = self.screen_width
        base_len = self.base_len

        img = np.zeros((height, width),dtype=np.uint8)
        img.fill(255)
        self.display_object(img, np.array([base_len, base_len]), base_len)
        self.display_object(img, np.array([base_len, height - 8 * base_len]), base_len)
        self.display_object(img, np.array([width - 8 * base_len, base_len]), base_len)
        self.display_object(img, np.array([width - 8 * base_len, height - 8 * base_len]), base_len)
        
        cv2.namedWindow("calibrate", cv2.WINDOW_NORMAL)
        cv2.setWindowProperty("calibrate", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
        cv2.imshow("calibrate", img)
        cv2.waitKey(30)
 
    # 计算pt0->pt1和pt0->pt2间的余弦值
    def cal_angle(self, pt1, pt2, pt0):
        pt0 = pt0[0]
        pt1 = pt1[0]
        pt2 = pt2[0]
        
        dx1 = pt1[0] - pt0[0]
        dy1 = pt1[1] - pt0[1]
        dx2 = pt2[0] - pt0[0]
        dy2 = pt2[1] - pt0[1]

        return (dx1 * dx2 + dy1 * dy2) / math.sqrt((dx1 ** 2 + dy1 ** 2) * (dx2 ** 2 + dy2 ** 2) + 1e-10)

    # 检测是否是 回 字
    def is_identifier(self, img, contour):
        area_all = img.shape[0] *img.shape[1]

        contour = cv2.approxPolyDP(contour, cv2.arcLength(contour, True) * 0.05, True)

        # 如果不是四边形，或者面积过大都不是目标（ps：面积有可能是负数）
        if len(contour) !=4 or math.fabs(cv2.contourArea(contour, False)) > 0.1 * area_all:
            return False
        
        # 计算交界处的最大余弦值，如果是矩形，其余弦值应该很小
        max_cosine = 0.0
        for i in range(2,5):
            cosine = math.fabs(self.cal_angle(contour[i % 4], contour[i - 2], contour[i - 1]))
            max_cosine = max(max_cosine, cosine)
        if max_cosine > 0.3:
            return False

        return True

    # 返回检测到的 回 字轮廓
    def find_identifier(self, img):
        # 边缘检测比二值化的抗光照效果更好
        img_binary = cv2.Canny(img, 100, 255, 3)
        img_binary = cv2.dilate(img_binary, np.ones((3, 3), np.uint8), iterations=3)
        img_binary = cv2.erode(img_binary, np.ones((3, 3), np.uint8), iterations=3)
        contours, hierarchy = cv2.findContours(img_binary, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        # 根据轮廓信息来判断目标
        parent_id = -1
        ic = 0
        child_id = -1
        result = []
        hierarchy = hierarchy[0]
        for i in range(0, len(hierarchy)):
            child_id = hierarchy[i][2]
            if child_id != -1 and ic == 0:
                parent_id = i
                ic += 1
            # 回 字中间的轮廓没有兄弟轮廓，且有且只有一个轮廓
            elif child_id != -1:
                ic += 1
            else:
                ic = 0
                parent_id = -1

            # 当前父轮廓有两个自轮廓，需要做进一步的判断
            if ic == 2:
                if self.is_identifier(img, contours[i]):
                    cv2.drawContours(img, contours, parent_id, (255, 0, 0), 2)
                    cv2.drawContours(img, contours, i, (255, 0, 0), 2)
                    cv2.drawContours(img, contours, hierarchy[i][2], (255, 0, 0), 2)
                    result.append(contours[i]) 
                ic = 0 
                parent_id = -1
        return result, img

    # 计算显示器坐标
    def cal_screen_coords(self, positions):
        result = []
        contours = np.zeros((4,2))
        contours_sorted = np.zeros((4, 2))

        if len(positions) != 8:
            return result
        
        # 计算四个 回 字的中心坐标
        for i in range(8):
            for j in range(len(positions[i])):
                contours[i // 2] += positions[i][j][0]
            contours[i // 2] /= len(positions[i])

        # 将四个坐标排序，按左上、左下、右下、右上的顺序 
        # 算法缺陷（需要保证显示器位于摄像头画面的中心位置）
        for i in range(4):
            if contours[i][0] < self.img_width / 2:
                if contours[i][1] < self.img_height / 2:    # 左上
                    contours_sorted[0] = contours[i]
                else:   # 左下
                    contours_sorted[1] = contours[i] 
            else:
                if contours[i][1] > self.img_height / 2:    # 右下
                    contours_sorted[2] = contours[i]
                else:   # 右上
                    contours_sorted[3] = contours[i]
        result.append(contours_sorted.astype(int))

        return result

   
    # 标定
    def calibrate(self, cap):
        if type(cap) == int or type(cap) == str:
            cap = cv2.VideoCapture(cap)
        elif type(cap) == cv2.VideoCapture:
            pass
        else:
            print("illegal input")
            return

        if not cap.isOpened():
            print("Error: can't open camera!")
            return

        # 在显示器上显示标定用的图片
        self.display()
        cnt = 0        

        # 如果100次都无法检测到目标，则超时
        while cap.isOpened() and cnt < 100:
            success, frame = cap.read()
            if not success:
                print("Error: cap.read failed!")
                return
            
            # 寻找符合条件的轮廓
            self.img_height = frame.shape[0]
            self.img_width = frame.shape[1]
            position, img = self.find_identifier(frame)     

            # 当检测到四个 回 字坐标后
            if len(position) == 8:
                contours = self.cal_screen_coords(position)        # 计算轮廓的坐标
                #cv2.drawContours(frame, contours, 0, (255, 0, 0), 2)

                # 计算映射到显示器的单应矩阵
                self.ratio = np.array([[(self.screen_width - 9 * self.base_len) / self.img_width],
                        [(self.screen_height - 9 * self.base_len) / self.img_height], [1]])

                dst_contours = np.array([[0, 0], [0, self.img_height], 
                    [self.img_width, self.img_height], [self.img_width, 0]])
                self.H = cv2.findHomography(np.array(contours), dst_contours)
                if not test:
                    break

        
            if self.H != None:
                print("reflection")
                frame = cv2.warpPerspective(frame, self.H[0], (self.img_width, self.img_height))

            if test:
                cv2.imshow("img", frame)
                cv2.waitKey(30)
            else:
                cnt += 1
        cv2.destroyAllWindows()
        if cnt == 100:
            print("Error: calibrate failed, timeout!")
        else:
            print("calibrate success")

    # 计算在屏幕上的坐标
    def reflect_to_screen(self, coords):
        x = np.array([[coords[0]], [coords[1]], [1]],dtype=np.float)
        x = self.H[0].dot(x) * (self.ratio) + 4.5 * self.base_len
        result = [0, 0]
        if x[0][0] < 0:
            result[0] = 0 
        elif x[0][0] > self.screen_width:
            result[0] = self.screen_width
        else:
            result[0] = int(x[0][0])

        if x[1][0] < 0:
            result[1] = 0
        elif x[1][0] > self.screen_height:
            result[1] = self.screen_height
        else:
            result[1] = int(x[1][0])
        
        return result

test = False 
if __name__ == "__main__":
    test = True
    calibrater = Calibrater()    
    calibrater.calibrate(0)
