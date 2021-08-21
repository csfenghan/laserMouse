/*
 * 本文件定义了屏幕标定算法的接口类
 * */
#ifndef CALIBRATE_HPP
#define CALIBRATE_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <pthread.h>
#include "config.hpp"

namespace lasermouse {
class Calibrater {
public:
    /* description: 输入当前屏幕的分辨率
     * param
     *     height: 显示器的高度分辨率
     *     width: 显示器的宽度分辨率
     * */
    explicit Calibrater(int screen_height, int screen_width, int grid_cols = 13, int grid_rows= 4);
    Calibrater();

    /* description: 使用配置类Config配置标定信息
     * */
    void setupConfig(const Config &conf);


    
    /* description: 标定当前屏幕与相机的位置
     * param 
     *     cap: 传入当前相机的视频流
     * return: 标定成功返回true，失败返回false
     * */
    bool calibrate(cv::VideoCapture &cap);


    /* description: 将摄像头拍摄到的图片中的坐标，变换到在显示器中的坐标
     * param
     *    coords: 输入要进行转换的坐标 
     * return: 变换后的坐标
     * */
    std::vector<std::vector<int>> coordsTransform(const std::vector<std::vector<int>> &coords);


    /* description: 功能测试
     * param:
     *     source: 使用的摄像头
     * */
    void test(int source);

private:
    /* description: 设置、恢复摄像头配置
     * */
    void setupCamera(cv::VideoCapture &cap);
    void resumeCamera(cv::VideoCapture &cap);


    /* description: 根据当前显示器的大小产生一个棋盘图片
     * */
    cv::Mat createChessboard();


    /*description: 生成圆形标定板
     * */
    cv::Mat createCicleGrid();

    int screen_height_;    // 显示器高度分辨率
    int screen_width_;     // 显示器宽度分辨率
    int grid_cols_;   // 棋盘角点的列数
    int grid_rows_;   // 棋盘角点的行数
    cv::Mat H_;     // 映射矩阵

    int auto_exposure_; // 相机自动曝光参数
    int exposure_;      // 相机曝光参数

    pthread_mutex_t mutex_;
};
} // namespace lasermouse

#endif
