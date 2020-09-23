#ifndef TESTENV_H
#define TESTENV_H

#include "opencv2/highgui/highgui.hpp"

int showExampleImg() {
	cv::Mat img = cv::imread("example.jpg", -1);
	if (img.empty()) return -1;

	cv::namedWindow("ImgTest", cv::WINDOW_AUTOSIZE);
	cv::imshow("ImgTest", img);
	cv::waitKey(0);
	cv::destroyWindow("ImgTest");
	return 0;
}

int scanQRCode(int argc, char* argv[]);

#endif 
// TESTENV_H