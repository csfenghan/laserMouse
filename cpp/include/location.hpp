#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <opencv2/opencv.hpp>
#include <pthread.h>
#include "calibrate.hpp"
#include "detect.hpp"
#include "config.h"

namespace lasermouse {
class Location {
  public:
      Location(int source = -1);
      explicit Location(const Config& conf);

      /* description:配置参数
       * */
      void setupConfig(const Config& conf);

      /* cescription:初始化
       * */
      void init();
      
      /* description: 标定当前位置
       * */
      void calibrate();

      /* description: 获取当前激光点在屏幕上的位置
       * param:
       *     x: 返回的x坐标
       *     y: 返回的y坐标
       * return:
       *     true: 发现激光点
       *     false: 没有发现激光点
       * */
      bool position(int &x, int &y);

  private:
      int camera_source_;
      pthread_mutex_t lock_;
      cv::VideoCapture cap_;

      Calibrater calibrater_;
      Detector detector_;
};
}

#endif
