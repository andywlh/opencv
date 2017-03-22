// personDetect3_22.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

bool isAnything = false;

struct Status{
	Status(int _framenum, Point _center, Rect _rect)
		:framenum(_framenum), center(_center), rect(_rect)
	{

	}
	int framenum;
	Point center;
	Rect rect;

};


bool biggerSort(vector<Point> v1, vector<Point> v2)
{
	return contourArea(v1) > contourArea(v2);
}

void filterContours(vector<vector<Point>> &contours){
	if (contours.empty()) return;
	sort(contours.begin(), contours.end(), biggerSort);
	for (int k = 0; k < contours.size(); ++k){
		if (contourArea(contours[k]) < contourArea(contours[0]) / 5)
		{
			contours.erase(contours.begin() + k,contours.end());
			break;
		}
		if (contourArea(contours[k]) < 300)
		{
			contours.erase(contours.begin() + k, contours.end());
			break;
		}
	}
	
}

void draw(const vector<Status> &objs,Mat img){
	for (int i = 0; i < objs.size(); ++i){
		line(img, objs[i].center, objs[i].center, 1, 8, 0);
	}
	imshow("objs", img);
}

int main()
{
	VideoCapture video("E:\\project\\PersonDetect3_22\\1.WMV");
	Mat frame,mask,bw,se;
	video >> frame;
	BackgroundSubtractorMOG2 bgSubtractor(20, 16, true);
	for (int i = 0; i < 50; ++i)
	{
		cout << "正在训练背景:" << i << endl;
		video >> frame;
		if (frame.empty() == true)
		{
			cout << "视频帧太少，无法训练背景" << endl;
			getchar();
			return 0;
		}
		bgSubtractor(frame, mask, 0.005);
	}

	Rect rt;
	se = getStructuringElement(MORPH_RECT, Size(5, 5));


	vector<Status> objs;

	//统计目标直方图时使用到的变量  
	vector<Mat> vecImg;
	vector<int> vecChannel;
	vector<int> vecHistSize;
	vector<float> vecRange;
	Mat m(frame.rows, frame.cols, DataType<uchar>::type);
	//变量初始化  
	vecChannel.push_back(0);
	vecHistSize.push_back(32);
	vecRange.push_back(0);
	vecRange.push_back(180);

	Mat hsv;        //HSV颜色空间，在色调H上跟踪目标（camshift是基于颜色直方图的算法）  
	MatND hist;     //直方图数组  
	double maxVal;      //直方图最大值，为了便于投影图显示，需要将直方图规一化到[0 255]区间上  
	Mat backP;      //反射投影图  
	Mat result;     //跟踪结果  
	Mat result2;
	int framenum = 1;
	while (1){
		framenum++;
		cout << framenum << endl;
		video >> frame;
		Mat background;
		bgSubtractor.getBackgroundImage(background);
		frame.copyTo(result);
		frame.copyTo(result2);
		bgSubtractor(frame, mask, 0.001);
		medianBlur(mask, mask, 5);
		//morphologyEx(mask, mask, MORPH_DILATE, getStructuringElement(MORPH_RECT, Size(5, 5)));
		imshow("video", frame);
		imshow("mask", mask);

		//检索前景中各个连通分量的轮廓  
		mask.copyTo(bw);
		vector<vector<Point>> contours;
		findContours(bw, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		filterContours(contours);
		if (contours.size() < 1)
		{
			waitKey(30);
			continue;
		}

		//对连通分量进行排序  
		//std::sort(contours.begin(), contours.end(), biggerSort);
	

		//结合camshift更新跟踪位置（由于camshift算法在单一背景下，跟踪效果非常好；  
		//但是在监控视频中，由于分辨率太低、视频质量太差、目标太大、目标颜色不够显著  
		//等各种因素，导致跟踪效果非常差。  因此，需要边跟踪、边检测，如果跟踪不够好，  
		//就用检测位置修改  
		cvtColor(frame, hsv, COLOR_BGR2HSV);
		vecImg.clear();
		vecImg.push_back(hsv);
		for (int k = 0; k < contours.size(); ++k)
		{
			//第k个连通分量的外接矩形框  
			//if (contourArea(contours[k]) < contourArea(contours[0]) / 5)
			//	break;
			//if (contourArea(contours[k]) < 300)
			//	break;
			rt = boundingRect(contours[k]);

			Point center(rt.x + (rt.width / 2), rt.y + (rt.height / 2));
			Status s(framenum,center,rt);
			objs.push_back(s);
			//cout << center.x << ' ' << center.y << endl;

			m = 0;
			m(rt) = 255;

			//统计直方图  
			calcHist(vecImg, vecChannel, m, hist, vecHistSize, vecRange);
			minMaxLoc(hist, 0, &maxVal);
			hist = hist * 255 / maxVal;
			//计算反向投影图  
			calcBackProject(vecImg, vecChannel, hist, backP, vecRange, 1);
			//camshift跟踪位置  
			Rect search = rt;
			//rectangle(result2, rt, Scalar(0, 255, 0), 2);
			//imshow("跟踪效果2", result2);
			RotatedRect rrt = CamShift(backP, search, TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 10, 1));
			Rect rt2 = rrt.boundingRect();
			rt &= rt2;

			//跟踪框画到视频上  
			rectangle(result, rt, Scalar(0, 255, 0), 2);
		}

		imshow("跟踪效果", result);
		//draw(objs, background);
		if (waitKey(4)=='27') break;;

	}

	
	getchar();
	return 0;
}

