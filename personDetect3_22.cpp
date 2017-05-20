// personDetect3_22.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include <fstream>
using namespace cv;
using namespace std;

bool isAnything = false;

struct Status{
	Status(int _framenum, Point _center, Rect _rect)
		:framenum(_framenum), center(_center), rect(_rect)
	{
		id = 0;
	}
	int framenum;//帧数
	int id;//目标编号
	Point center;//形心坐标
	Rect rect;//包围矩形


};//找出的目标在某一帧中的信息

Scalar colors[6] = { (0, 255, 255), (205, 173, 0), (238, 99, 99), (160, 32, 240), (205, 175, 149), (144, 238, 144) };

vector<Status> findStatusByframe(vector<Status> const &objs, int framenum); //通过帧数查找status
void filterContours(vector<vector<Point>> &contours);//过滤contours，删除过小的结果
Mat draw(vector<Status> &objs, Mat img);//根据保存的运动物体信息画出运动轨迹
int dist(Point x, Point y);//判断两个中心点的距离
bool sortByframe(Status &objs1, Status &objs2);//通过帧数先后对status排序
bool isSameObj(vector<Status> objs, Status &status);//判断两个Status是否属于同一个目标
void classifyObjs(vector<Status> &objs);//对整个vector<Status>进行分类，添加id
vector<Status> findStatusById(vector<Status> const &objs, int id);//通过id查找Status
bool isPerson(Mat img);

bool biggerSort(vector<Point> v1, vector<Point> v2)
{
	return contourArea(v1) > contourArea(v2);
}

void filterContours(vector<vector<Point>> &contours){
	if (contours.empty()) return;			//如果为空则返回
	sort(contours.begin(), contours.end(), biggerSort); //对contours按面积大小降序排序
	for (int k = 0; k < contours.size(); ++k){
		if (contourArea(contours[k]) < contourArea(contours[0]) / 5) //若contours小于最大的contours的1/5则删除
		{
			contours.erase(contours.begin() + k,contours.end()); 
			break;
		}
		if (contourArea(contours[k]) < 300)		//若contours面积小于则删除
		{
			contours.erase(contours.begin() + k, contours.end());
			break;
		}
	}
	
}

Mat draw( vector<Status> &objs,Mat img){
	int maxId = 0;
	for (int i = 0; i < objs.size(); ++i){
		if (maxId < objs[i].id) maxId = objs[i].id;
	}

	for (int i = 0; i <= maxId; ++i){
		vector<Status> s =findStatusById(objs, i);
		for (int j = 0; j < s.size(); ++j){
			if (j < s.size() - 1) line(img, s[j].center, s[j + 1].center, colors[i % 5], 4, 8, 0);
		}
	}

	//for (int i = 0; i < objs.size(); ++i){
	//	if(i<objs.size()-1&&objs[i].id==objs[i+1].id) line(img, objs[i].center, objs[i+1].center, colors[objs[i].id%5], 4, 8, 0);//根据id选择随机颜色，画出轨迹
	//}
	if(!img.empty()) imshow("objs", img);
	return img;
}

int dist(Point x,Point y)//求两点之间的距离
{
	int distance;
	double res;
	res = (x.x - y.x) * (x.x - y.x) + (x.y - y.y) * (x.y - y.y);
	distance = (int)(sqrt(res));
	return distance;
}

bool sortByframe(Status &objs1, Status &objs2){
	return objs1.framenum < objs2.framenum;
		
}

vector<Status> findStatusByframe(vector<Status> const &objs, int framenum){
	vector<Status> result;
	for (int i=0; i < objs.size(); ++i){
		if (objs[i].framenum == framenum)
			result.push_back(objs[i]);
	}
	return result;
}

vector<Status> findStatusById(vector<Status> const &objs, int id){
	vector<Status> res;
	for (int i = 0; i < objs.size(); ++i){
		if (objs[i].id == id)
			res.push_back(objs[i]);
	}
	return res;
}

bool isSameObj(vector<Status> objs, Status &status){
	
	bool t = status.center.x > 100 && status.center.y>100&&status.center.x<380&&status.center.y<540;
	for (int i = 0; i < objs.size(); ++i){
		if (dist(status.center, objs[i].center) < 50&&(status.framenum-objs[i].framenum)<48){
			status.id = objs[i].id;
			return true;
		}
	}
	return false;
}

void classifyObjs(vector<Status> &objs){
	sort(objs.begin(), objs.end(), sortByframe);
	int frm = objs[0].framenum;

	int _id = 1;

	for (int i = 0; i < objs.size(); ++i){
		if (objs[i].framenum == frm){//初始化时将找到的目标赋予初始id
			objs[i].id = _id;
			_id++;
		}
	}
	static int  __id = _id;
	for (int i = 0; i < objs.size(); ++i){
		if (objs[i].id!=0) continue;
		vector<Status> preObjs = findStatusByframe(objs, objs[i].framenum - 1);
		if (isSameObj(preObjs, objs[i])) continue;//判断是否属于同一物体
			
		objs[i].id = __id;
		__id++;//若是新物体则赋予新的id，并让id自加
		
	}
}



int personDetect(string videoUrl){
	VideoCapture video(videoUrl);
	Mat frame, mask, bw, se, resultpic;
	
	

	if (frame.empty())
	{
		printf("loading video failed!\n");
		//return -1;
	}
	video >> frame;
	long nFrame = static_cast<long>(video.get(CV_CAP_PROP_FRAME_COUNT));
	int frameRate1 = video.get(CV_CAP_PROP_FPS);
	int framenum1 = video.get(CV_CAP_PROP_FRAME_COUNT);
	int width1 = (int)video.get(CV_CAP_PROP_FRAME_WIDTH);
	int height1 = (int)video.get(CV_CAP_PROP_FRAME_HEIGHT);
	Size frameSize = Size(width1, height1);
	VideoWriter writer1;
	
	string title = to_string(nFrame);
	//string t = now_time;
	//t.append("result.avi");

	writer1.open(title+"result.avi", CV_FOURCC('D', 'I', 'V', 'X'), frameRate1, frameSize, true);

	BackgroundSubtractorMOG2 bgSubtractor(20, 16, true);  //高斯背景建模初始化，用到初始帧数20，马克平方数16
	for (int i = 0; i < 50; ++i)  //使用视频的初始帧训练背景
	{
		cout << "正在训练背景:" << i << endl;
		video >> frame;
		if (frame.empty()) continue;
		if (frame.empty() == true)
		{
			cout << "视频帧太少，无法训练背景" << endl;
			getchar();
			return 0;
		}
		bgSubtractor(frame, mask, 0.005);
	}

	Rect rt;
	//se = getStructuringElement(MORPH_RECT, Size(5, 5));


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
	double maxVal;  //直方图最大值，为了便于投影图显示，需要将直方图规一化到[0 255]区间上  
	Mat backP;      //反射投影图  
	Mat result;     //跟踪结果  
	Mat result2;
	int framenum = 51;
	while ((nFrame - 10)>framenum){
		framenum++;					//帧数自加
		cout << framenum << endl;
		video >> frame;				//从视频流中导入一帧到frame
		if (frame.empty()) continue;
		Mat background;				//用于保存视频背景
		bgSubtractor.getBackgroundImage(background); //获取视频背景
		frame.copyTo(result);		//复制frame到result，在result上绘制轨迹
		frame.copyTo(result2);
		bgSubtractor(frame, mask, 0.001);  //调用MOG2获取背景
		medianBlur(mask, mask, 5);        //进行中值滤波，去除噪声影响
		if (!frame.empty()) imshow("原始视频", frame);
		if (!mask.empty())imshow("前景提取", mask);

		//检索前景中各个连通分量的轮廓  
		mask.copyTo(bw);
		vector<vector<Point>> contours;
		findContours(bw, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		filterContours(contours);  //对所得到的contours进行过滤，删掉太小的
		if (contours.size() < 1)
		{
			waitKey(30);
			continue;
		}




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
			Status s(framenum, center, rt);
			objs.push_back(s);
			classifyObjs(objs);
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
			
			rectangle(result2, rt, Scalar(0, 255, 0), 2);
			//putText(result2, to_string(objs.back().id), s.center, 1, 5, Scalar(0, 0, 255));
			
			
			//if (!result2.empty())imshow("人形标记", result2);
			RotatedRect rrt = CamShift(backP, search, TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 10, 1));
			Rect rt2 = rrt.boundingRect();
			rt &= rt2;

			//跟踪框画到视频上  
			rectangle(result, rt, Scalar(0, 255, 0), 2);
			//putText(result, to_string(objs.back().id), s.center, 1, 5, Scalar(0, 0, 255));
		}

		if (!result.empty())imshow("跟踪效果", result);
		
		writer1.write(result);
		resultpic=draw(objs, background);
		if (waitKey(4) == 'q') break;

	}
	imwrite(title+"result.jpg", resultpic);
	ofstream in;
	in.open(title+ "result.txt", ios::trunc);
	
	for (int i = 0; i < objs.size(); ++i){
		in << objs[i].framenum << ' ' << objs[i].id << ' ' << objs[i].center.x << ' ' << objs[i].center.y << ' ' << objs[i].rect.height << ' ' << objs[i].rect.width << endl;
	}
	in.close();

}

int main(int argc, char *argv[])
{
	//string videoUrl = "E:\\project\\personDetect3_22\\12F-FONT1.WMV";
	string videoUrl = "E:\\project\\video\\5.17\\刀闸\\5月12号早上7点40刀闸由闭转开_剪辑.avi";
	personDetect(videoUrl);
	
	return 0;
}

