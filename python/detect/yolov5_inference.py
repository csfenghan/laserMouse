"""
已废弃的检测方法，在投影仪与LCD显示器上检测效果均不佳。
使用yolov5来检测输入的图片
"""
import os
import sys
sys.path.append(os.getcwd())

import cv2
import onnxruntime
import torch
import numpy as np
import torchvision

# 推理模型
class LaserDetector():
    def __init__(self, onnx_path, inference_shape=(640, 640), input_shape=None):
        self.onnx_session = onnxruntime.InferenceSession(onnx_path)

        self.inference_shape = [inference_shape[0], inference_shape[1]]  # onnx模型的输入大小
        self.input_shape = [input_shape[0], input_shape[1]] if input_shape is not None else None   # 输入的图片的大小
        self.unpad_shape = None
        self.ratio = None
        self.pad = None # （tp, bottom, left, right）

        # 计算缩放填充尺度
        self.cal_scale(input_shape)

    # 计算需要缩放与填充的尺度
    # 输入[h, w]
    def cal_scale(self, img_shape):
        if img_shape is None:
            return

        new_shape = self.inference_shape
        old_shape = img_shape

        # 计算缩放比例
        r = min(new_shape[0] / old_shape[0], new_shape[1] / old_shape[1])
        self.ratio = r

        # 计算需要pad的大小
        unpad_shape = [int(round(old_shape[0] * r)), int(round(old_shape[1] * r))]
        pad_h, pad_w = new_shape[0] - unpad_shape[0], new_shape[1] - unpad_shape[1]
        self.unpad_shape = unpad_shape

        # 填充
        pad_h /= 2
        pad_w /= 2
        
        self.pad = [int(round(pad_h - 0.1)), int(round(pad_h + 0.1)), 
                int(round(pad_w - 0.1)), int(round(pad_w + 0.1))]


    # 对输入的图片进行预处理
    def pre_process(self, img):
        if self.input_shape != img.shape[:2]:
            self.cal_scale(img.shape[:2]) 
        
        # 缩放填充图片
        img = cv2.resize(img, (self.unpad_shape[1], self.unpad_shape[0]), interpolation=cv2.INTER_LINEAR) 
        img = cv2.copyMakeBorder(img, self.pad[0], self.pad[1], self.pad[2], self.pad[3], cv2.BORDER_CONSTANT, value=0)
        # 预处理
        img = img[:, :, ::-1].transpose(2, 0, 1) # BGR to RGB 
        img = img.astype(dtype=np.float32)
        img /= 255.0
        img = np.expand_dims(img, axis=0)

        return img

     # 将坐标从[x, y, w, h]转换成[x1, y1, x2, y2]
    def xywh2xyxy(self, x):
        y = np.copy(x)

        y[:, 0] = x[:, 0] - x[:, 2] / 2  # top left x
        y[:, 1] = x[:, 1] - x[:, 3] / 2  # top left y
        y[:, 2] = x[:, 0] + x[:, 2] / 2  # bottom right x
        y[:, 3] = x[:, 1] + x[:, 3] / 2  # bottom right y

        return y

    # 执行nm操作
    def nms(self, prediction, conf_thres=0.1, iou_thres=0.6, max_det=300, agnostic=False):
        if prediction.dtype is torch.float16:
            prediction = prediction.float()  # to FP32
        xc = prediction[..., 4] > conf_thres  # candidates
        min_wh, max_wh = 2, 4096  # (pixels) minimum and maximum box width and height
        output = [None] * prediction.shape[0]
        for xi, x in enumerate(prediction):  # image index, image inference
            x = x[xc[xi]]  # confidence
            if not x.shape[0]:
                continue

            x[:, 5:] *= x[:, 4:5]  # conf = obj_conf * cls_conf
            box = self.xywh2xyxy(x[:, :4])

            conf, j = x[:, 5:].max(1, keepdim=True)
            x = torch.cat((torch.tensor(box), conf, j.float()), 1)[conf.view(-1) > conf_thres]
            n = x.shape[0]  # number of boxes
            if not n:
                continue
            c = x[:, 5:6] * (0 if agnostic else max_wh)  # classes
            boxes, scores = x[:, :4] + c, x[:, 4]  # boxes (offset by class), scores
            i = torchvision.ops.boxes.nms(boxes, scores, iou_thres)
            if i.shape[0] > max_det:  # limit detections
                i = i[:max_det]
            output[xi] = x[i]

        return output

    # 将坐标映射到原来的图片上
    # 当有多个的时候取第一个
    def scale_coords(self, coords):
        coords[0][0][0] -= self.pad[2]
        coords[0][0][2] -= self.pad[2]
        coords[0][0][1] -= self.pad[0]
        coords[0][0][3] -= self.pad[0]

        coords[0][0] *= self.ratio

        result = []
        result.append([int(coords[0][0][0].item()), int(coords[0][0][1].item()), 
            int(coords[0][0][2].item()), int(coords[0][0][3].item())])
        return result


    # 输入opencv类型图片，输出位置
    def run(self, img):
        # 预处理
        h, w, c = img.shape
        x = self.pre_process(img)

        # 推理
        y = {self.onnx_session.get_inputs()[0].name: x}
        y = self.onnx_session.run(None, y)

        # NMS
        y = torch.Tensor(y[0])
        pred = self.nms(y, conf_thres=0.2, iou_thres=0.4)
        
        # 处理结果
        result = []
        if pred[0] is None:
            return result

        result = self.scale_coords(pred)
        return result
        
        for i, det in enumerate(pred):
            if not len(det):
                continue
            det[:, :4] = scale_coords(self.inference_shape, det[:, :4], (h, w, c)).round()
            for *xyxy, conf, cls in reversed(det):
                result.append([(int)(xyxy[0].item()), (int)(xyxy[1].item()), 
                    (int)(xyxy[2].item()), (int)(xyxy[3].item()), conf.item()])
        return result

if __name__ == "__main__":
    pass
