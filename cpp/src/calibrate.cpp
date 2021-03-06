#include "calibrate.hpp"
#include <iostream>

namespace lasermouse{
/* description: 输入当前屏幕的分辨率
* param
*     height: 显示器的高度分辨率
*     width: 显示器的宽度分辨率
* */
Calibrater::Calibrater(int screen_height, int screen_width, int grid_cols, int grid_rows):
    screen_height_(screen_height), screen_width_(screen_width), grid_cols_(grid_cols), grid_rows_(grid_rows),
    auto_exposure_(3), exposure_(30), mutex_(PTHREAD_MUTEX_INITIALIZER) {}

Calibrater::Calibrater():screen_height_(-1), screen_width_(-1), grid_cols_(-1), grid_rows_(-1), 
                    auto_exposure_(3), exposure_(30), mutex_(PTHREAD_MUTEX_INITIALIZER) {}

/* description: 使用配置类Config配置标定信息
* */
void Calibrater::setupConfig(const Config &conf) {
    pthread_mutex_lock(&mutex_);

    screen_width_ = conf.screen_width;
    screen_height_ = conf.screen_height;
    grid_rows_ = conf.grid_rows;
    grid_cols_ = conf.grid_cols;

    pthread_mutex_unlock(&mutex_);
}

/* description: 根据当前显示器的大小产生一个棋盘图片
 * */
cv::Mat Calibrater::createChessboard() {
    int n_pix = MIN(screen_height_, screen_width_) / 10;  // 一个块的像素数
    int pad_h = screen_height_ - n_pix * (grid_rows_+ 1);    
    int pad_w = screen_width_ - n_pix * (grid_cols_+ 1);

    int row_start = pad_h / 2, row_end = screen_height_ - (pad_h - pad_h / 2); 
    int col_start = pad_w / 2, col_end = screen_width_ - (pad_w - pad_w / 2);

    // 在中心位置创建棋盘图片
    cv::Mat result = cv::Mat(screen_height_, screen_width_, CV_8UC1, cv::Scalar::all(255));
    for (int i = row_start; i < row_end; i++) {
        for (int j = col_start; j < col_end; j++) {
            if ((((i - row_start) / n_pix) & 0x01) == 0) {  // 如果是偶数行
                if ((((j - col_start) / n_pix) & 0x01) != 0) {
                    result.at<uchar>(i, j) = 0;
                }
            } else {    // 如果是奇数行
                if ((((j - col_start) / n_pix) & 0x01) == 0) {
                    result.at<uchar>(i, j) = 0;
                }
            }
        }
    }

    return result;
}

/*
 * description: 生成圆形标定板
 * */
cv::Mat Calibrater::createCicleGrid() {
    int n_pix = MIN(screen_height_, screen_width_) / 10;  // 一个块的像素数
    int pad_h = screen_height_ - n_pix * (2 * grid_rows_+ 1);    
    int pad_w = screen_width_ - n_pix * (grid_cols_+ 1);
    int radius = n_pix / 3;

    int row_start = pad_h / 2; 
    int col_start = pad_w / 2;

    // 在中心位置创建棋盘图片
    cv::Mat result = cv::Mat(screen_height_, screen_width_, CV_8UC1, cv::Scalar::all(255));
    int row = row_start + n_pix;
    for (int i = 0; i < 2 * grid_rows_; i++) {
        int col = col_start + n_pix;
        for (int j = 0; j < grid_cols_; j++) {
            if ((i & 0x01) == 0){   // 如果是偶数行
                if ((j & 0x01) != 0)
                    cv::circle(result, cv::Point(col, row), radius, cv::Scalar(0), -1);
            } else {
                if ((j & 0x01) == 0) 
                    cv::circle(result, cv::Point(col, row), radius, cv::Scalar(0), -1);
            }
            col += n_pix;
        }
        row += n_pix;
    }
    return result;
}

/* description: 标定当前屏幕与相机的位置
* param 
*     cap: 传入当前相机的视频流
* return: 标定成功返回true，失败返回false
* */
bool Calibrater::calibrate(cv::VideoCapture &cap) {
    if (!cap.isOpened()) {
        std::cerr << "cap open failed" << std::endl;
        return false;
    }
    setupCamera(cap);

    // 全屏显示图片
    cv::Mat grid_show= createCicleGrid();

    cv::namedWindow("calibrate", cv::WINDOW_NORMAL);
    cv::setWindowProperty("calibrate", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    cv::imshow("calibrate", grid_show);
    cv::waitKey(100);

    // 开始标定
    cv::Mat frame;
    std::vector<cv::Point2d> corners_show, corners_detect;
    int times = 30;
    while (cap.isOpened()) {
        if (times-- == 0)
            break;
        if (cap.read(frame) == false)
            break;

        // 检测角点并匹配
        if (!cv::findCirclesGrid(grid_show, cv::Size(grid_rows_, grid_cols_), 
                    corners_show, cv::CALIB_CB_ASYMMETRIC_GRID)) {
            printf("detect failed\n");
        }
        if (cv::findCirclesGrid(frame, cv::Size(grid_rows_, grid_cols_), 
                    corners_detect, cv::CALIB_CB_ASYMMETRIC_GRID)) {
            pthread_mutex_lock(&mutex_);
            H_ = cv::findHomography(corners_detect, corners_show);
            pthread_mutex_unlock(&mutex_);
            break;
        }
    }

    cv::destroyWindow("calibrate");
    resumeCamera(cap);
    if (H_.empty() && times <= 0) {
        std::cerr << "calibrate failed, timeout!" << std::endl;
        return false;
    }
    std::cout << "calibrate successed" << std::endl;
    std::cout << "H = " << cv::format(H_, cv::Formatter::FMT_NUMPY) << std::endl;
    return true;
}

/* description: 将摄像头拍摄到的图片中的坐标，变换到在显示器中的坐标
* param
*    coords: 输入要进行转换的坐标 
* return: 变换后的坐标
* */
std::vector<std::vector<int>> Calibrater::coordsTransform(const std::vector<std::vector<int>> &coords) {
    std::vector<std::vector<int>> result; 

    // 将输入转换成Mat数组
    cv::Mat X = cv::Mat(3, coords.size(), CV_64FC1, cv::Scalar::all(0)); 
    for (size_t i = 0; i < coords.size(); i++) {
        X.at<double>(i, 0) = coords[i][0];
        X.at<double>(i, 1) = coords[i][1];
        X.at<double>(i, 2) = 1;
    }

    // 坐标变换
    pthread_mutex_lock(&mutex_);
    cv::Mat Y = H_ * X;
    pthread_mutex_unlock(&mutex_);

    // 将输出转换成vector
    for (int i = 0; i < Y.cols; i++) {
        std::vector<int> temp;
        temp.push_back(int(Y.at<double>(0, i) / Y.at<double>(2, i)));
        temp.push_back(int(Y.at<double>(1, i) / Y.at<double>(2, i)));
        result.push_back(temp);
    }
    return result;
}

/* description: 设置、恢复摄像头配置
* */
void Calibrater::setupCamera(cv::VideoCapture &cap) {
    auto_exposure_ = cap.get(cv::CAP_PROP_AUTO_EXPOSURE);    
    exposure_ = cap.get(cv::CAP_PROP_EXPOSURE);    

    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
    cap.set(cv::CAP_PROP_EXPOSURE, 30);
}
void Calibrater::resumeCamera(cv::VideoCapture &cap) {
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, auto_exposure_);
    cap.set(cv::CAP_PROP_EXPOSURE, exposure_);
}
/* description: 功能测试
* param:
*     source: 使用的摄像头
* */
void Calibrater::test(int source) {
    auto cap = cv::VideoCapture(source);
    cv::Mat frame;

    setupCamera(cap);
    if (!calibrate(cap)) 
        return;

    cv::namedWindow("frame", cv::WINDOW_NORMAL);
    cv::resizeWindow("frame", 500, 500);
    while (cap.isOpened()) {
        cap.read(frame);
        cv::warpPerspective(frame, frame, H_, cv::Size(screen_width_, screen_height_));
        cv::imshow("frame", frame);
        cv::waitKey(20);
    }
    resumeCamera(cap);
}
}
