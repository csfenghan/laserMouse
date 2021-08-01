#include "location.hpp"
#include <iostream>

namespace lasermouse {
Location::Location(int source):calibrater_(1080, 1920), detector_(){
    cap_.open(source);
    detector_.setupCamera(cap_); 
}
/* description: 标定当前位置
* */
void Location::calibrate() {
    calibrater_.setupCamera(cap_);
    calibrater_.calibrate(cap_);
    calibrater_.resumeCamera(cap_);
}

/* description: 获取当前激光点在屏幕上的位置
* param:
*     x: 返回的x坐标
*     y: 返回的y坐标
* return:
*     true: 发现激光点
*     false: 没有发现激光点
* */
bool Location::position(int &x, int &y) {
    cv::Mat frame;  
    if (!cap_.read(frame)) {
        std::cerr << "Read camera failed" << std::endl;
        return false;
    } 
    auto coords = detector_.detect(frame, 0.5);
    if (coords.empty())
        return false;

    auto screen_coords = calibrater_.coordsTransform(coords);
    x = screen_coords[0][0];
    y = screen_coords[0][1];
    return true;
}
}