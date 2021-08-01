#include "detect.hpp"
#include <numeric>

namespace lasermouse{
/*
* param:
*     screen_type: 显示器的类型,projector:投影仪   lcd:lcd显示器
*     screen_height: 屏幕高度分辨率
*     screen_width: 屏幕宽度分辨率
*     camera_height: 摄像头高度分辨率
*     camera_width: 摄像头宽度分辨率
* */
Detector::Detector(ScreenType screen_type, int screen_height, int screen_width,
        int camera_height, int camera_width):screen_type_(screen_type), screen_height_(screen_height),
        screen_width_(screen_width), camera_height_(camera_height), camera_width_(camera_width), 
        auto_exposure_(3), exposure_(30){}

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
    
    switch (screen_type_) {
        case PROJECTOR:
            cap.set(cv::CAP_PROP_EXPOSURE, 1);
            break;
        case LCD:
            cap.set(cv::CAP_PROP_EXPOSURE, 30);
            break; 
        default:
            std::cerr << "unknow screen type: " << screen_type_ << std::endl;
    }
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
