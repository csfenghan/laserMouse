#include "location.hpp"
#include "mouse_control.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    lasermouse::Location location; 
    lasermouse::MouseControl mouse_control;

    int x = 0, y = 0;
    location.calibrate();
    while (true) {
        if (location.position(x, y)){
            mouse_control.moveTo(x, y);
        }
    }
}
