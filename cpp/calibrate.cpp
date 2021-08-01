#include "calibrate.hpp"

namespace lasermouse{
Calibrater::Calibrater(int height, int width, int chessboard_cols, int chessboard_rows):
	height_(height), width_(width), chessboard_cols_(chessboard_cols), chessboard_rows_(chessboard_rows) {}

cv::Mat Calibrater::createChessboard() {
	int n_pix = MIN(height_, width_) / 10;	// 一个块的像素数
	int col = n_pix * (chessboard_cols_ + 1);
	int row = n_pix *(chessboard_rows_ + 1);
	
	// 创建棋盘图片
	cv::Mat result = cv::Mat(row, col, CV_8UC1, cv::Scalar::all(0));
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			if (((i / n_pix) & 0x01) == 0) {	// 如果是偶数行
				if (((j / n_pix) & 0x01) != 0) {
					result.at<uchar>(i, j) = 255;
				}
			} else { 	// 如果是奇数行
				if (((j / n_pix) & 0x01) == 0) {
					result.at<uchar>(i, j) = 255;
				}
			}
		}
	}
	
	return result;
}
}
