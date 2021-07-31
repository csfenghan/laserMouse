"""
本文件负责对Detector检测到的目标进行追踪、平滑
"""
from detect.threshold_detector import Detector
import numpy as np

class Tracker:
    """
    输入坐标信息，追踪目标,单目标追踪
    """
    def __init__(self, delay_cnt=10):
        """
        :param delay_cnt: 当检测不到目标时，维持目标的时间
        """
        self.object = []    # 正在追踪的目标
        self.delay_cnt = delay_cnt
        self.curr_cnt = 0

    def update(self, coords):
        """
        :param coords: 新检测到的坐标(只追踪一个目标)
        """
        if len(coords) == 0:
            if not len(self.object) == 0:
                self.curr_cnt += 1
                if self.curr_cnt >= self.delay_cnt:
                    self.object.remove(self.object[0])
        else:
            self.object = [coords[0]]
            self.curr_cnt = 0

    def position(self):
        """
        获取当前的位置坐标
        """
        return self.object
