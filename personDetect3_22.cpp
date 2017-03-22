// personDetect3_22.cpp : �������̨Ӧ�ó������ڵ㡣
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
		cout << "����ѵ������:" << i << endl;
		video >> frame;
		if (frame.empty() == true)
		{
			cout << "��Ƶ̫֡�٣��޷�ѵ������" << endl;
			getchar();
			return 0;
		}
		bgSubtractor(frame, mask, 0.005);
	}

	Rect rt;
	se = getStructuringElement(MORPH_RECT, Size(5, 5));


	vector<Status> objs;

	//ͳ��Ŀ��ֱ��ͼʱʹ�õ��ı���  
	vector<Mat> vecImg;
	vector<int> vecChannel;
	vector<int> vecHistSize;
	vector<float> vecRange;
	Mat m(frame.rows, frame.cols, DataType<uchar>::type);
	//������ʼ��  
	vecChannel.push_back(0);
	vecHistSize.push_back(32);
	vecRange.push_back(0);
	vecRange.push_back(180);

	Mat hsv;        //HSV��ɫ�ռ䣬��ɫ��H�ϸ���Ŀ�꣨camshift�ǻ�����ɫֱ��ͼ���㷨��  
	MatND hist;     //ֱ��ͼ����  
	double maxVal;      //ֱ��ͼ���ֵ��Ϊ�˱���ͶӰͼ��ʾ����Ҫ��ֱ��ͼ��һ����[0 255]������  
	Mat backP;      //����ͶӰͼ  
	Mat result;     //���ٽ��  
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

		//����ǰ���и�����ͨ����������  
		mask.copyTo(bw);
		vector<vector<Point>> contours;
		findContours(bw, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		filterContours(contours);
		if (contours.size() < 1)
		{
			waitKey(30);
			continue;
		}

		//����ͨ������������  
		//std::sort(contours.begin(), contours.end(), biggerSort);
	

		//���camshift���¸���λ�ã�����camshift�㷨�ڵ�һ�����£�����Ч���ǳ��ã�  
		//�����ڼ����Ƶ�У����ڷֱ���̫�͡���Ƶ����̫�Ŀ��̫��Ŀ����ɫ��������  
		//�ȸ������أ����¸���Ч���ǳ��  ��ˣ���Ҫ�߸��١��߼�⣬������ٲ����ã�  
		//���ü��λ���޸�  
		cvtColor(frame, hsv, COLOR_BGR2HSV);
		vecImg.clear();
		vecImg.push_back(hsv);
		for (int k = 0; k < contours.size(); ++k)
		{
			//��k����ͨ��������Ӿ��ο�  
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

			//ͳ��ֱ��ͼ  
			calcHist(vecImg, vecChannel, m, hist, vecHistSize, vecRange);
			minMaxLoc(hist, 0, &maxVal);
			hist = hist * 255 / maxVal;
			//���㷴��ͶӰͼ  
			calcBackProject(vecImg, vecChannel, hist, backP, vecRange, 1);
			//camshift����λ��  
			Rect search = rt;
			//rectangle(result2, rt, Scalar(0, 255, 0), 2);
			//imshow("����Ч��2", result2);
			RotatedRect rrt = CamShift(backP, search, TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 10, 1));
			Rect rt2 = rrt.boundingRect();
			rt &= rt2;

			//���ٿ򻭵���Ƶ��  
			rectangle(result, rt, Scalar(0, 255, 0), 2);
		}

		imshow("����Ч��", result);
		//draw(objs, background);
		if (waitKey(4)=='27') break;;

	}

	
	getchar();
	return 0;
}

