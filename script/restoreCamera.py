"""
恢复摄像头自动曝光
"""
import cv2

if __name__ == "__main__":
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 3)
