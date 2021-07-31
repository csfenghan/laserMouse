/*
 * 录制视频
 * */

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char **argv) {
    int fourcc, frame_rate;
    char c;
    Mat frame;
    VideoCapture cap;
    VideoWriter writer;
    Size frame_size;
    bool is_recording;

    //如果没有指定摄像头，则打开默认的摄像头
    if (argc == 1) {
        cap.open(0);
    } else {
        cap.open(atoi(argv[1]));
    }

    if (!cap.isOpened()) {
        cerr << "video not open!" << endl;
        exit(0);
    }

    //配置保存的视频格斯
    fourcc = writer.fourcc('M', 'J', 'P', 'G');
    frame_rate = cap.get(CAP_PROP_FPS);
    frame_size = Size(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
    writer.open("save.avi", writer.fourcc('M', 'J', 'P', 'G'), frame_rate, frame_size, true);

    //录制
	cout<<"press 's' to start record"<<endl;
	cout<<"press 'e' to end record"<<endl;
	cout<<"press 'q' to quit"<<endl;

    is_recording = false;
    while (cap.isOpened()) {
        if (!cap.read(frame)) {
            cerr << "videos read error" << endl;
            break;
        }

        imshow("img", frame);
        if (is_recording) {
            writer.write(frame);
        }

		//捕获按键
        c = waitKey(1000 / frame_rate);
        if (c == 'q') {
            break;
        } else if (c == 's') {
            is_recording = true;
        } else if (c == 'e') {
            is_recording = false;
        }
    }
    cap.release();
    writer.release();
    destroyAllWindows();

    exit(0);
}
