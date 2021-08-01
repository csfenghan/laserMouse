#include "calibrate.hpp"
#include "detect.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    auto calibrater = lasermouse::Calibrater(1080, 1920);
    calibrater.test(0);

    //auto detector = lasermouse::Detector();
    //detector.test(0);
}
