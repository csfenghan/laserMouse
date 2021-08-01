#ifndef DETECT_HPP
#define DETECT_HPP

#include <opencv2/opencv.hpp>
#include <vector>

namespace lasermouse {

// 显示器的种类
enum ScreenType {
    PROJECTOR = 1,  // 投影仪
    LCD = 2         // lcd显示器
};

class Detector {
 public:
    /*
     * param:
     *     screen_type: 显示器的类型,PROJECTOR:投影仪   LCD:lcd显示器
     *     screen_height: 屏幕高度分辨率
     *     screen_width: 屏幕宽度分辨率
     *     camera_height: 摄像头高度分辨率
     *     camera_width: 摄像头宽度分辨率
     * */
    Detector(ScreenType screen_type = PROJECTOR, int screen_height = 1080, int screen_width = 1920,
            int camera_height = 480, int camera_width = 640);

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
  private:
    ScreenType screen_type_;    // 屏幕类型
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
