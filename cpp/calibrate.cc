/*
 * 该文件用于识别屏幕四个顶点的识别码，并给出屏幕的映射矩阵
 * */

#include <math.h>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

//计算pt0->pt1和pt0->pt2之间的余弦值
double cal_angle(Point pt1, Point pt2, Point pt0) {
        double dx1, dy1, dx2, dy2;

        dx1 = pt1.x - pt0.x;
        dy1 = pt1.y - pt0.y;
        dx2 = pt2.x - pt0.x;
        dy2 = pt2.y - pt0.y;

        return (dx1 * dx2 + dy1 * dy2) /
               sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

//检测到底是不是“回”字
bool is_identifier(Mat img, vector<Point> &contour) {
        vector<Point> contour_approx;
        int area_all = img.cols * img.rows;
        double max_cosine, cosine;

        approxPolyDP(contour, contour_approx, arcLength(contour, true) * 0.05, true);

        //如果不是四边形，或者面积过大都不是目标;根据轮廓方向，面积有可能是负的
        if (contour_approx.size() != 4 || fabs(contourArea(contour, false)) > 0.1 * area_all)
                return false;

        //计算交接边缘间角度的最大余弦值，如果最大的余弦值都非常小，则认为是矩形
        max_cosine = 0;
        for (int j = 2; j < 5; j++) {
                cosine = fabs(
                    cal_angle(contour_approx[j % 4], contour_approx[j - 2], contour_approx[j - 1]));
                max_cosine = max(max_cosine, cosine);
        }
        if (max_cosine > 0.3)
                return false;

        return true;
}

//返回检测到的“回”字的轮廓
vector<vector<Point>> find_identifier(Mat img) {
        Mat img_gray, img_binary, kernel;
        vector<vector<Point>> contours;
        vector<vector<Point>> result;
        vector<Vec4i> hierarchy;

        //二值化后寻找轮廓
        /*cvtColor(img, img_gray, COLOR_BGR2GRAY);
            equalizeHist(img_gray,img_gray);
        threshold(img_gray, img_binary, 80, 255, THRESH_BINARY );
        findContours(img_binary, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0,
        0));*/
        //使用边缘检测代替二值化可以更好的抗光照
        Canny(img, img_binary, 100, 255, 3);
        dilate(img_binary, img_binary, Mat(), Point(-1, -1), 3);
        erode(img_binary, img_binary, Mat(), Point(-1, -1), 3);
        findContours(img_binary, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
        //imshow("binary", img_binary);

        //根据“回”字的轮廓等级来判断目标
        int parent_id = -1, ic = 0, child_id = -1;
        for (size_t i = 0; i < hierarchy.size(); i++) {
                child_id = hierarchy[i][2];
                if (child_id != -1 && ic == 0) {
                        parent_id = i;
                        ic++;
                }
                //“回”字的中间的轮廓应该没有兄弟轮廓，也只有一个子轮廓，用这个来进行筛选，但初步筛选不用太复杂
                else if (child_id != -1) {
                        ic++;
                } else {
                        ic = 0;
                        parent_id = -1;
                }

                //当前父轮廓有两个子轮廓，需要做进一步的判断（机器学习or其他）
                if (ic == 2) {
                        if (is_identifier(img, contours[i])) {
                                drawContours(img, contours, parent_id, Scalar(0, 0, 255), 2);
                                drawContours(img, contours, i, Scalar(0, 0, 255), 2);
                                drawContours(img, contours, hierarchy[i][2], Scalar(0, 0, 255), 2);

                                result.push_back(contours[i]);
                        }
                        ic = 0;
                        parent_id = -1;
                }
        }

        return result;
}

//计算显示器的坐标
vector<vector<Point>> filter_identifier(vector<vector<Point>> &position) {
        vector<Point> contours(4, Point(0, 0));
        vector<vector<Point>> result;

        if (position.size() != 8)
                return result;

        //左上、左下、右上、右下坐标
        for (int i = 0; i < 8; i++) {
		for (size_t j = 0; j < position[i].size(); j++){
			contours[i / 2].x += position[i][j].x;
			contours[i / 2].y += position[i][j].y;
		}
                contours[i / 2].x /= position[i].size();
                contours[i / 2].y /= position[i].size();
        }
        result.push_back(contours);

        return result;
}

//计算映射的矩阵

int main(int argc, char **argv) {
        Mat frame;
        VideoCapture cap;
        vector<vector<Point>> contours;

        //如果没有指定视频路径的话，默认打开摄像头
        if (argc == 1) {
                cap.open(0);
        } else {
                cap.open(atoi(argv[1]));
        }

        //处理视频信息
        while (cap.isOpened()) {
                if (!cap.read(frame)) {
                        cerr << "read video error" << endl;
                        exit(0);
                }

                // 1.寻找满足特征的“回”字的轮廓
                vector<vector<Point>> position = find_identifier(frame);
		cout << position.size() << endl;
                // 2.根据找到的“回”字判断是否是要寻找的目标，如果有则筛选出其坐标
                if (position.size() == 8) {
                        contours = filter_identifier(position);
                        drawContours(frame, contours, 0, Scalar(0, 0, 255), 2);
                }

                // 3.根据四个顶点的位置来计算映射矩阵
                imshow("Img", frame);

                if (waitKey(30) == 'q') {
                        break;
                }
        }
        cap.release();
        destroyAllWindows();
}
