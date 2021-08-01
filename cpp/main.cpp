#include "calibrate.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
	lasermouse::Calibrater calibrater = lasermouse::Calibrater(1080, 1920);

	cv::Mat img = calibrater.createChessboard();

	printf("%d, %d", img.rows, img.cols);
	cv::imshow("img", img);
	if (cv::waitKey(0) == 'q')
		return 0;
}
