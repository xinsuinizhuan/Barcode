#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/imgproc/types_c.h>
#include "decoder/ean_decoder.hpp"

namespace cv {
    void test() {
        std::cout << __DATE__ << " " << __TIME__ << '\n';
        std::cout.flush();
        std::string imgPath = R"(..\test\data\K71NM_45.jpg)";
        cv::Mat greyImg = cv::imread(imgPath, IMREAD_GRAYSCALE);

        cv::Mat thresholdImg;
        cv::threshold(greyImg, thresholdImg, 127, 255, CV_THRESH_OTSU);
        int height = thresholdImg.rows;

        int center = height / 2;
        cv::imshow("2zhi", thresholdImg);
        //cv::waitKey();

        cv::Mat centerLine = thresholdImg.rowRange(center, center + 1);
        std::vector<uchar> middle_array = centerLine.isContinuous() ? centerLine : centerLine.clone();
        //cv::imshow("center line", centerLine);
        //cv::waitKey();
        ean_decoder decoder = ean_decoder(EAN13);
        decoder.rect_to_ucharlist(greyImg, vector<RotatedRect>());
        std::cout << decoder.decode(middle_array, 0) << std::endl;
        cv::waitKey();
    }


}