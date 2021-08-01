#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <opencv2/opencv.hpp>
#include "calibrate.hpp"
#include "detect.hpp"

namespace lasermouse {
class Location {
  public:
      Location(int source = 0);

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
      cv::VideoCapture cap_;
      Calibrater calibrater_;
      Detector detector_;
};
}

#endif
