#include "detect.hpp"
#include <numeric>

namespace lasermouse{
/*
* param:
*     screen_height: 屏幕高度分辨率
*     screen_width: 屏幕宽度分辨率
*     camera_height: 摄像头高度分辨率
*     camera_width: 摄像头宽度分辨率
* */
Detector::Detector(int screen_height, int screen_width,
        int camera_height, int camera_width):screen_height_(screen_height),
        screen_width_(screen_width), camera_height_(camera_height), camera_width_(camera_width), 
        auto_exposure_(3), exposure_(30){}
Detector::Detector():screen_height_(-1), screen_width_(-1), camera_height_(-1), camera_width_(-1),
        auto_exposure_(3), exposure_(30) {}
/*
* param:
*    conf:配置类
* */
Detector::Detector(const Config &conf) {
    setupConfig(conf);
}

/*
* param:
*    conf:配置类
* */
void Detector::setupConfig(const Config &conf) {
    camera_height_ = conf.camera_height;
    camera_width_ = conf.camera_width;
    screen_height_ = conf.screen_height;
    screen_width_ = conf.screen_width;
}

/* description: 设置相机分辨率、曝光、fps
* param:
*     cap: 相机的视频流
* */
void Detector::setupCamera(cv::VideoCapture &cap) {
    auto_exposure_ = cap.get(cv::CAP_PROP_AUTO_EXPOSURE);
    exposure_ = cap.get(cv::CAP_PROP_EXPOSURE);

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, camera_height_);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, camera_width_);
    cap.set(cv::CAP_PROP_FPS, 30);
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
    
    cap.set(cv::CAP_PROP_EXPOSURE, 1);
}

void Detector::resumeCamera(cv::VideoCapture &cap) {
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, auto_exposure_);
    cap.set(cv::CAP_PROP_EXPOSURE, exposure_);
}

/* description: 检测图片中激光点的坐标
* param:
*     img: 输入的图片
*     conf: 筛选目标用的置信度，conf越高，筛选越严格
* */
std::vector<std::vector<int>> Detector::detect(cv::Mat img, double conf) {
    int thresh = int(255 * conf); 
    std::vector<cv::Point2d> locations;
    std::vector<std::vector<int>> result;

    // 灰度化、二值化、寻找坐标
    cv::cvtColor(img, img, cv::COLOR_RGB2GRAY);
    cv::threshold(img, img, thresh, 255, cv::THRESH_BINARY);
    cv::findNonZero(img, locations); 
    if (locations.empty())
        return result;

    // 默认只有一个目标，累计求均值
    int x, y;
    x = std::accumulate(locations.begin(), locations.end(), 0, 
            [](int a, cv::Point2d b){return a + b.x;});
    y = std::accumulate(locations.begin(), locations.end(), 0, 
            [](int a, cv::Point2d b){return a + b.y;});
    x /= locations.size();
    y /= locations.size();
    result.push_back(std::vector<int>{x, y}); 

    return result;
}

/* description: 功能测试
* param:
*     source: 使用的摄像头
* */
void Detector::test(int source) {
    cv::VideoCapture cap = cv::VideoCapture(source);
    cv::Mat frame;

    setupCamera(cap);
    while (cap.isOpened()) {
        cap.read(frame);    
        auto coords = detect(frame);
        if (!coords.empty())
            cv::circle(frame, cv::Point2d(coords[0][0], coords[0][1]), 5, cv::Scalar(255, 0, 0));
        cv::imshow("frame", frame);
        cv::waitKey(30);
    } 
    resumeCamera(cap);
}
}
