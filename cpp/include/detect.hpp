#ifndef DETECT_HPP
#define DETECT_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include "config.hpp"

namespace lasermouse {

class Detector {
 public:
    /*
     * param:
     *     screen_height: 屏幕高度分辨率
     *     screen_width: 屏幕宽度分辨率
     *     camera_height: 摄像头高度分辨率
     *     camera_width: 摄像头宽度分辨率
     * */
    Detector(int screen_height, int screen_width,
            int camera_height, int camera_width);
    Detector();
    explicit Detector(const Config &conf);

    /*
     * param:
     *    conf:配置类
     * */
    void setupConfig(const Config &conf);

    /* description: 设置、恢复相机分辨率、曝光、fps
     * param:
     *     cap: 相机的视频流
     * */
    void setupCamera(cv::VideoCapture &cap);
    void resumeCamera(cv::VideoCapture &cap);

    /* description: 检测图片中激光点的坐标
     * param:
     *     img: 输入的图片
     *     conf: 筛选目标用的置信度，conf越高，筛选越严格
     * */
    std::vector<std::vector<int>> detect(cv::Mat img, double conf = 0.5);

    /* description: 功能测试
     * param:
     *     source: 使用的摄像头
     * */
    void test(int source);
  private:
    int screen_height_;
    int screen_width_;
    int camera_height_;
    int camera_width_;

    // 保存的相机参数
    int auto_exposure_;
    int exposure_;
};
} // namespace lasermouse

#endif
