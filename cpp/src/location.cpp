#include "location.hpp"
#include <iostream>

namespace lasermouse {

Location::Location(int source){}

/* description:配置参数
* */
void Location::setupConfig(const Config& conf) {
    camera_source_ = conf.camera_source;
    calibrater_.setupConfig(conf);
    detector_.setupConfig(conf);
}

/* description:初始化
 * */
void Location::init() {
    pthread_mutex_init(&lock_, NULL);
    cap_.open(camera_source_);
    if (!cap_.isOpened()) {
        std::cerr << "failed to open camera" << std::endl;
        exit(-1);
    }
    detector_.setupCamera(cap_); 
}

/* description: 标定当前位置
* */
void Location::calibrate() {
    pthread_mutex_lock(&lock_);

    calibrater_.calibrate(cap_);

    pthread_mutex_unlock(&lock_);
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
    pthread_mutex_lock(&lock_);
    if (!cap_.read(frame)) {
        pthread_mutex_unlock(&lock_);
        std::cerr << "Read camera failed" << std::endl;
        return false;
    } 
    pthread_mutex_unlock(&lock_);

    auto coords = detector_.detect(frame, 0.5);
    if (coords.empty())
        return false;

    auto screen_coords = calibrater_.coordsTransform(coords);
    x = screen_coords[0][0];
    y = screen_coords[0][1];
    return true;
}
}
