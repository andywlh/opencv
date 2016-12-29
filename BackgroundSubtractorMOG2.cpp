// BackgroundSubtractorMOG2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

void useMask(Mat img, Mat mask, Mat result) {
	int cols = img.cols;
	int rows = img.rows;
	for (int x = 0; x < cols; x++) {
		for (int y = 0; y < rows; y++) {
			if (mask.at<uchar>(x, y) == 0xff) result.at<uchar>(x, y) = img.at<uchar>(x, y);
		}
	}


}

int main()
{
	VideoCapture video("video.avi");

	long totalFrameNumber = video.get(CV_CAP_PROP_FRAME_COUNT);
	Mat frame, mask, thresholdImage, output,background;
	video>>frame;  
	VideoWriter foreground("foreground.avi", CV_FOURCC('M', 'J', 'P', 'G'),24.0,Size(frame.cols,frame.rows));
	BackgroundSubtractorMOG2 bgSubtractor(480, 16, true);
	int frameNum = 0;
	cout << frame.rows << endl;
	cout << frame.cols << endl;
	while (frameNum<totalFrameNumber) {	
		if (frameNum >= totalFrameNumber - 1000) break;
		//cout << totalFrameNumber << endl;
		video >> frame;
		++frameNum;
		bgSubtractor(frame, mask, 0.001); 
		cout << frameNum << endl;
		bgSubtractor.getBackgroundImage(background);
		
		erode(mask, mask, Mat());
		dilate(mask, mask, Mat());
		
		int n = 0;
		for (int j = 0; j<mask.cols; j++)
			for (int i = 0; i<mask.rows; i++) {
				int s = mask.at<uchar>(i, j);
				if (s == 0xff) n++;
			}
		if (n > 50) {
			//printf("found person!!!");
			foreground << frame;
		}

	
		
		if(!mask.empty()) imshow("mask",mask);
		if(!frame.empty()) imshow("video", frame);
		if(!background.empty()) imshow("background", background);
		if (waitKey(20) == 27) break;
	}
    return 0;
}

