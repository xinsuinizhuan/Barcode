#ifndef BARCODE_INCLUDE_TEST_ENV_H
#define BARCODE_INCLUDE_TEST_ENV_H

#include "opencv2/highgui/highgui.hpp"

int showExampleImg() {
    cv::Mat imgdata;
    imgdata.create(214, 315, CV_32FC1);
    cv::randn(imgdata, cv::Scalar::all(0), cv::Scalar::all(1));
    //cv::Mat img = cv::imread("example.jpg", -1);
    //if (img.empty()) return -1;
    cv::namedWindow("ImgTest", cv::WINDOW_AUTOSIZE);
    cv::imshow("ImgTest", imgdata);
    cv::waitKey(0);
    cv::destroyWindow("ImgTest");
    return 0;
}

#endif //! BARCODE_INCLUDE_TEST_ENV_H