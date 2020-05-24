/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#include<stdio.h>
#include<iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sys/stat.h> 
#include <sys/types.h>
#include<ctime>
#include <sys/time.h>
#include <OpenNI.h>
#include "Viewer.h"
#include "opencv/highgui.h" 
#include "opencv2/imgproc/imgproc_c.h" 
#include "opencv2/core/core.hpp" 
#include "opencv2/imgproc/imgproc.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/calib3d/calib3d.hpp"

#define SAMPLE_READ_WAIT_TIMEOUT 500 //2000ms
#define MAX_PATH_LEN 256

#ifdef _MKDIR_LINUX
#define ACCESS(fileName,accessMode) _access(fileName,accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName,accessMode) access(fileName,accessMode)
#define MKDIR(path) mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

bool protonect_shutdown = false; // Whether the running application should shut down.

using namespace std;
using namespace openni;
using namespace cv;

int FrameToDepth(const VideoFrameRef& frame, Mat& depth1)
{ 
	uint16_t *pPixel; 
	for (int y = 0; y < frame.getHeight(); y++) 
	{ 

		pPixel = ((uint16_t*)((char*)frame.getData() + ((int)(y)* frame.getStrideInBytes()))); 
		uint16_t* data = (uint16_t*)depth1.ptr<uchar>(y); 
		for (int x = 0; x < frame.getWidth(); x++) 
		{ 
			*data++ = (*pPixel); 
			 pPixel++; 
		} 
	} 
	return 0; 
}

void WriteLog(double RunTime, double NotStartStream1, double WaitFailed1, double NotStartStream2, double WaitFailed2, double RunNum) 
{ 
	struct timeval tv;
	gettimeofday(&tv, NULL);
	FILE *fp;
	fp = fopen("log.txt", "at"); 
	//fprintf(fp, "MyLogInfo: %d:%d:%d:%d\n ", tv.wHour, tv.wMinute, tv.wSecond, tv.wMilliseconds); 
	fprintf(fp, "运行时间：%f\n", RunTime); fprintf(fp, "运行次数：%f \n", RunNum); 
	fprintf(fp, "设备1流开始失败次数：%f 设备1等待失败次数：%f \n", NotStartStream1, WaitFailed1);
	fprintf(fp, "设备2流开始失败次数：%f 设备2等待失败次数：%f \n", NotStartStream2, WaitFailed2); 
	fclose(fp); 
}

// 从左到右依次判断文件夹是否存在,不存在就创建
// example: /home/root/mkdir/1/2/3/4/
// 注意:最后一个如果是文件夹的话,需要加上 '\' 或者 '/'
int32_t createDirectory(const std::string &directoryPath)
{
    cout<<"\n开始创建文件夹\n";
    uint32_t dirPathLen = directoryPath.length();
    if (dirPathLen > MAX_PATH_LEN)
    {
        return -1;
    }
    char tmpDirPath[MAX_PATH_LEN] = { 0 };
    for (uint32_t i = 0; i < dirPathLen; ++i)
    {
        tmpDirPath[i] = directoryPath[i];
        if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
        {
            if (ACCESS(tmpDirPath, 0) != 0)
            {
                int32_t ret = MKDIR(tmpDirPath);
                if (ret != 0)
                {
                    return ret;
                }
            }
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
	struct timeval tv;
	long Time0 = 0; 
	long Time1 = 0; 
	int Time11 = 0; 
	int Time22 = 0; 
	double Sum = 0;
	gettimeofday(&tv, NULL);
	time_t t1;
	time_t t2;
	time(&t1);
	double NotStartStream1 = 0; //设备1不能打开流的次数 
	double NotStartStream2 = 0; //设备2不能打开流的次数 
	double WaitFailed1 = 0; //设备1等待超时的次数 
	double WaitFailed2 = 0; //设备1等待超时的次数
	double Time_frame = 0;
	bool SHOW_ON = false;//默认关闭显示
	//std::stringstream &filename;
	//std::string & filename;
	if (argc > 1)
	{
		std::cout << argv[1] << endl;
	}
	
	char status_show[10];
	std::cout << "是否显示:[yes|no]" << std::endl;
	cin>>status_show;
	if(strcmp(status_show, "yes") == 0) //两者相等则返回0，否则为其他值
	{
		SHOW_ON = true;//显示图片视频
	}
	else if(strcmp(status_show, "no") == 0)
		{
			SHOW_ON = false;//不显示图片视频
		}
		else
		{
			cout<<"\n输入信息有误\n";
			return -1;
		}
		
	//创建文件夹
	string file_address(argv[1], argv[1] + strlen(argv[1]));
	std::string SaveDepthAddress = file_address+"depth/";
	std::string SaveRGBAddress = file_address+"rgb/";
	if (argc == 2)
	{
		int return_num = createDirectory(argv[1]);//创建储存主文件夹
		createDirectory(SaveDepthAddress);//创建储存深度图文件夹
		createDirectory(SaveRGBAddress);//创建储存rgb图文件夹
	}
	else
	{
		cout<<"\n输入信息有误,不能创建文件夹\n";
		return -1;
	}
	
	openni::Status rc = openni::STATUS_OK;//初始化状态变量
	openni::Device device;
	openni::VideoFrameRef m_RGB_frame;
	openni::VideoFrameRef m_depth_frame;
	openni::VideoStream depth, color;
	const char* deviceURI = openni::ANY_DEVICE;
	cv::Mat frame,frameResized;//视频帧
	cv::Mat depth2(480, 640, CV_16UC1);//深度图转换
	
	//初始化函数
	rc = openni::OpenNI::initialize();
	if (rc != openni::STATUS_OK) 
	{ 
		printf("Initialize failed\n%s\n", openni::OpenNI::getExtendedError()); //报告错误
		return 1; 
	} 
	printf("After initialization:\n%s\n", openni::OpenNI::getExtendedError());

	//设置镜像
	
	
	//开启设备
	rc = device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		printf("Astra_test: Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();//关闭设备
		return 1;
	}

	//创建depth_IR流 
	rc = depth.create(device, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK)
	{
		rc = depth.start();//开始深度流
		if (rc != openni::STATUS_OK)
		{
			printf("Astra_test: Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
			depth.destroy();
		}
		else
		{
			printf("Astra_test: Start depth stream: OK.\n\n");
		}
	}
	else
	{
		printf("Astra_test: Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	//创建RGB_IR流 	
	rc = color.create(device, openni::SENSOR_COLOR);
	if (rc == openni::STATUS_OK)
	{
		rc = color.start();//开始视频流
		if (rc != openni::STATUS_OK)
		{
			printf("Astra_test: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
			color.destroy();
		}
		else
		{
			printf("Astra_test: Start color stream: OK.\n\n");
		}
	}
	else
	{
		printf("Astra_test: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	if (!depth.isValid() || !color.isValid())
	{
		printf("Astra_test: No valid streams. Exiting\n");
		openni::OpenNI::shutdown();
		return 2;
	}

	//与视频深度帧的帧率相对应
	std::ofstream f1;
	string TS_file_address = file_address +"depth .txt";
	f1.open(TS_file_address, ios::out|ios::trunc);//覆盖，不存在则创建
	if(!f1.fail())
	{
		f1 << "# depth maps" << std::endl;
		f1 << "# file: 'Astra-s_###.zip'" << std::endl;
		f1 << "# timestamp filename" << std::endl;
	}
	f1 << std::fixed;
	
	//与视频的帧率相对应
	std::ofstream f;
	TS_file_address = file_address +"rgb.txt";
	f.open(TS_file_address, ios::out|ios::trunc);//覆盖，不存在则创建
	if(!f.fail())
	{
		f << "# color images" << std::endl;
		f << "# file: 'Astra-s_###.zip'" << std::endl;
		f << "# timestamp filename" << std::endl;
	}
	f << std::fixed;
		
	VideoStream* streams_RGB1 = &color; 
	VideoStream* streams_Depth =&depth; 
	int readyStream_RGB1 = -1;
	gettimeofday(&tv, NULL);
	Time0 = tv.tv_sec * 1000 + tv.tv_usec / 1000; // 毫秒
	while(!protonect_shutdown)
	{
		int readyStream_Depth = -1; 
		rc = OpenNI::waitForAnyStream(&streams_Depth, 1, &readyStream_Depth, SAMPLE_READ_WAIT_TIMEOUT); 
		if (rc != STATUS_OK) 
		{ 
			printf("Wait failed! (timeout is %d ms)\n%s\n", SAMPLE_READ_WAIT_TIMEOUT, OpenNI::getExtendedError());
			continue;
		}

		depth.readFrame(&m_depth_frame); //color1.readFrame(&frame1); 

		if (m_depth_frame.isValid())
		{ 
			FrameToDepth(m_depth_frame, depth2); 
			if( SHOW_ON == true)
			{
				namedWindow("Depth-Img",CV_WINDOW_AUTOSIZE);
				imshow("Depth-Img", depth2);
			}
			printf("Depth[%08llu]\n",(long long)m_depth_frame.getTimestamp());
			gettimeofday(&tv, NULL);
			
			Time_frame = m_depth_frame.getTimestamp();
			Time_frame = Time_frame/1000000.0;//转换为秒
			double current_Time = 2001021000.0;
			current_Time = current_Time+Time_frame;
			f1 << current_Time;
			f1 << " ";
			f1 << "depth/" << std::to_string(current_Time) << ".png" << std::endl;
			std::stringstream  DepthName;
			DepthName << std::to_string(current_Time) << ".png";
			string file_address(argv[1], argv[1] + strlen(argv[1]));
			std::string SaveFilesName = SaveDepthAddress+DepthName.str();
			cv::imwrite(SaveFilesName,depth2);
		}
		
	  	rc = OpenNI::waitForAnyStream(&streams_RGB1, 1, &readyStream_RGB1, SAMPLE_READ_WAIT_TIMEOUT); 
		if (rc != STATUS_OK)
		{
			printf("Wait failed! (timeout is %d ms)\n%s\n", SAMPLE_READ_WAIT_TIMEOUT, OpenNI::getExtendedError());
			continue;
		}
		color.readFrame(&m_RGB_frame); 
		//color1.readFrame(&frame1); 
		if (m_RGB_frame.isValid()) 
		{ 
			//将帧保存到Mat中并转换为BGR模式，因为在OpenCV中图片的模式是BGR	  
			const openni::RGB888Pixel* imageBuffer = (const openni::RGB888Pixel*)m_RGB_frame.getData();
			frame.create(m_RGB_frame.getHeight(), m_RGB_frame.getWidth(), CV_8UC3);
			memcpy(frame.data, imageBuffer, 3*m_RGB_frame.getHeight()*m_RGB_frame.getWidth()*sizeof(uint8_t));
			cvtColor(frame, frame, CV_RGB2BGR);
			if( SHOW_ON == true )
			{
				namedWindow("BGR-Img",CV_WINDOW_AUTOSIZE);
				imshow("BGR-Img", frame);
			}
			//printf("Astra_test: Image show...\n");
			printf("RGB[%08llu]\n",(long long)m_RGB_frame.getTimestamp());
			gettimeofday(&tv, NULL);
			Time1 = tv.tv_sec * 1000 + tv.tv_usec / 1000; // 毫秒
			Time_frame = m_RGB_frame.getTimestamp();
			Time_frame = Time_frame/1000000.0;//转换为秒
			double current_Time = 2001021000.0;
			current_Time = current_Time+Time_frame;
			f << current_Time;
			f << " ";
			f << "rgb/" << std::to_string(current_Time) << ".png" << std::endl;
			std::stringstream  ImageName;
			ImageName << std::to_string(current_Time) << ".png";
			std::string SaveFilesName = SaveRGBAddress+ImageName.str();
			int Image_size_Max = fmax(frame.rows,frame.cols);
			if(Image_size_Max > 640)
			{
				float scale = (float)640.0f/(float)Image_size_Max; 
				cv::resize(frame,frameResized,cv::Size(),scale,scale);
			}
			else	
				frameResized = frame;
			cv::imwrite(SaveFilesName,frameResized);
		} 

		gettimeofday(&tv, NULL);
		Time1 = tv.tv_sec * 1000 + tv.tv_usec / 1000; // 毫秒
		Time22 = Time1 - Time0; 
		std::cout << "设备花费时间为：" << Time22 << endl; 
		std::cout << "运行次数" << Sum << endl; 
		Sum++; 
		time(&t2);
		std::cout << "运行时间为：" << t2 - t1 << endl; 
		std::cout << endl;
		if ((int)Sum % 100 == 0) 
		{ 
		  WriteLog(t2 - t1, NotStartStream1, WaitFailed1, NotStartStream2, WaitFailed2, Sum);
		} 

		int key = cv::waitKey(1);
		protonect_shutdown = protonect_shutdown || (key > 0 && ((key & 0xFF) == 27)); // shutdown on escape
	}
	f1.close();
	f.close();
	//销毁显示窗口
	cv::destroyWindow("Depth-Img");
	cv::destroyWindow("RGB-Img");
	//关闭视频流
	depth.destroy();
	color.destroy();
	//关闭设备
	device.close();
	//关闭OpenNI
	openni::OpenNI::shutdown();
	//SampleViewer sampleViewer("Simple Viewer", device, depth, color);

	//rc = sampleViewer.init(argc, argv);
	//if (rc != openni::STATUS_OK)
	//{
	//	openni::OpenNI::shutdown();
	//	return 3;
	//}
	//sampleViewer.run();
}
