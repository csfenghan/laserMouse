#ifndef MOUSE_CONTROL_HPP
#define MOUSE_CONTROL_HPP

#ifdef _WIN32

#else
#include <X11/Xlib.h>
#endif

namespace lasermouse {
class MouseControl {
  public:
      MouseControl();

      /* description: 设置当前鼠标的坐标
       * param:
       *     x: 设置的x坐标
       *     y: 设置的y坐标
       * */
      void moveTo(int x, int y);

      /* description: 获取当前鼠标的坐标
       * param:
       *     x: 返回x坐标
       *     y: 返回y坐标
       * */
      void position(int &x, int &y); 
  private:
      /* description: x窗口初始化
       * */
      void init(); 

      Display *display;
      Window root;
};
}

#endif
