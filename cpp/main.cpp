#include "calibrate.hpp"
#include "detect.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    auto detector = lasermouse::Detector();
    detector.test(0);
}
