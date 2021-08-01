#include "calibrate.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
	lasermouse::Calibrater calibrater = lasermouse::Calibrater(1080, 1920);

	auto cap = cv::VideoCapture(0);

	calibrater.calibrate(cap);
}
