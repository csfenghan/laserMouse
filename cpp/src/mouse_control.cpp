#include "mouse_control.hpp"
#include <iostream>

namespace lasermouse {

MouseControl::MouseControl() {
    init();
}

/* description: x窗口初始化
* */
void MouseControl::init() {
    if ((display = XOpenDisplay(NULL)) == NULL) {
        std::cerr << "Cannot open local X-display." << std::endl;
        exit(-1);
    }
    root = DefaultRootWindow(display);

    pthread_mutex_init(&lock_, NULL); 
}

/* description: 设置当前鼠标的坐标
* param:
*     x: 设置的x坐标
*     y: 设置的y坐标
* */
void MouseControl::moveTo(int x, int y) {
    pthread_mutex_lock(&lock_);
    XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
    XFlush(display);
    pthread_mutex_unlock(&lock_);
}

/* description: 获取当前鼠标的坐标
* param:
*     x: 返回x坐标
*     y: 返回y坐标
* */
void MouseControl::position(int &x, int &y) {
    int  tmp;unsigned  int  tmp2;
    Window fromroot, tmpwin;

    pthread_mutex_lock(&lock_);
    XQueryPointer(display, root, &fromroot, &tmpwin, &x, &y, &tmp, &tmp, &tmp2);
    pthread_mutex_unlock(&lock_);
}
/* description:点击左键
* */
void MouseControl::clickLeft() {

}

/* description:点击右键
* */
void MouseControl::clickRight() {

}


}
