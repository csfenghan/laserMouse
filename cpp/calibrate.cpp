#include "calibrate.hpp"
#include <iostream>

namespace lasermouse{
Calibrater::Calibrater(int height, int width, int chessboard_cols, int chessboard_rows):
	height_(height), width_(width), chessboard_cols_(chessboard_cols), chessboard_rows_(chessboard_rows) {}

cv::Mat Calibrater::createChessboard() {
	int n_pix = MIN(height_, width_) / 10;	// 一个块的像素数
	int pad_h = height_ - n_pix * (chessboard_rows_+ 1);	
	int pad_w = width_ - n_pix * (chessboard_cols_+ 1);

	int row_start = pad_h / 2, row_end = height_ - (pad_h - pad_h / 2);	
	int col_start = pad_w / 2, col_end = width_ - (pad_w - pad_w / 2);

	// 在中心位置创建棋盘图片
	cv::Mat result = cv::Mat(height_, width_, CV_8UC1, cv::Scalar::all(255));
	for (int i = row_start; i < row_end; i++) {
		for (int j = col_start; j < col_end; j++) {
			if ((((i - row_start) / n_pix) & 0x01) == 0) {	// 如果是偶数行
				if ((((j - col_start) / n_pix) & 0x01) != 0) {
					result.at<uchar>(i, j) = 0;
				}
			} else { 	// 如果是奇数行
				if ((((j - col_start) / n_pix) & 0x01) == 0) {
					result.at<uchar>(i, j) = 0;
				}
			}
		}
	}

	return result;
}
}
