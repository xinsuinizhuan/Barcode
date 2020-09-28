#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<iostream>
#include<stdlib.h>
#include <opencv2\imgproc\types_c.h>
#include <iostream>
#include "AbsDecoder.h"
#include "EanDecoder.h"

void test() {
	std::string imgPath = "D:\\Project\\Barcode\\img\\a.jpg";
	cv::Mat rawImg = cv::imread(imgPath, -1);
	cv::Mat greyImg;
	
	cv::cvtColor(rawImg, greyImg, CV_RGBA2GRAY);

	cv::Mat thresholdImg;
	cv::threshold(greyImg, thresholdImg, 127, 255, CV_THRESH_OTSU);
	int width = thresholdImg.cols;
	int height = thresholdImg.rows;

	int center = height / 2;
	cv::imshow("2zhi", thresholdImg);
	cv::waitKey();
	cv::Mat centerLine = thresholdImg.rowRange(center, center + 1);
	int unit = 0;
	bool counting = false;
	int start = -1;
	for (int i = 0; i < centerLine.cols; i++) {
		int temp = centerLine.at<uchar>(0,i);
		if (temp == BLACK && !counting) {
			counting = true;
			start = i;
		}

		if (counting && temp == BLACK) {
			unit++;
		}
		else if(temp == WHITE && counting)
		{
			break;
		}
	}
	std::vector<uchar> array;
	array = (std::vector<uchar>)centerLine.reshape(1, 1);
	EanDecoder decoder = EanDecoder(EAN13, unit);
	std::cout << decoder.decode(array, start) << std::endl;

}