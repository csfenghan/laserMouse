/*
 * 本文件用于在显示器四个顶点生成“回”字图案，用于相机识别定位
 * */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

/* 在指定位置显示“回”字,该字的格式与QR二维码的三个顶点的“回”字格式相同 */
void display_object(Mat img, Point start, int base_len) {
    Point end(start.x + 7 * base_len, start.y + 7 * base_len);

    rectangle(img, start, end, Scalar(0, 0, 0), -1);

    start.x += base_len;
    start.y += base_len;
    end.x -= base_len;
    end.y -= base_len;
    rectangle(img, start, end, Scalar(255, 255, 255), -1);

    start.x += base_len;
    start.y += base_len;
    end.x -= base_len;
    end.y -= base_len;
    rectangle(img, start, end, Scalar(0, 0, 0), -1);
}

int main(int argc, char **argv) {
    // 1.获取屏幕尺寸，确定绘制“回”字的基本的单位长度
    const int height = 1650, width = 2200;
    const int basic_len = min(height, width) / 28;

    // 2.生成一张新的图片，在图片的四个顶点处分别绘制“回“字
    Mat image = Mat::zeros(height, width, CV_8UC3);

    image.setTo(255);
    display_object(image, Point(basic_len, basic_len), basic_len);
    display_object(image, Point(basic_len, height - 8 * basic_len), basic_len);
    display_object(image, Point(width - 8 * basic_len, basic_len), basic_len);
    display_object(image, Point(width - 8 * basic_len, height - 8 * basic_len),
                   basic_len);

    // 3.全屏显示图片
    namedWindow("calibrate", WINDOW_NORMAL);
    setWindowProperty("calibrate", WND_PROP_FULLSCREEN,
                      WINDOW_FULLSCREEN);

    imshow("calibrate", image);
    if (waitKey(0) == 'q') {
        imwrite("output.jpg", image);
        exit(0);
    }
    exit(0);
}
