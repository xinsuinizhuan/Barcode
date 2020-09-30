#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cstdlib>
#include <opencv2/imgproc/types_c.h>
#include <iostream>
#include <decoder/AbsDecoder.hpp>
#include "decoder/EanDecoder.h"
#include "picture_path.h"

namespace cv {
    void test() {
        std::cout << __DATE__ << " " << __TIME__ << '\n';
        std::cout.flush();
        std::string imgPath = test_picture_path;
        cv::Mat greyImg = cv::imread(imgPath, IMREAD_GRAYSCALE);
        //cv::Mat greyImg;
        //cv::cvtColor(rawImg, greyImg, CV_RGBA2GRAY);

        cv::Mat thresholdImg;
        cv::threshold(greyImg, thresholdImg, 127, 255, CV_THRESH_OTSU);
        //int width = thresholdImg.cols;
        int height = thresholdImg.rows;

        int center = height / 2;
        cv::imshow("2zhi", thresholdImg);
        //cv::waitKey();
        cv::Mat centerLine = thresholdImg.rowRange(center, center + 1);
        cv::imshow("center line", centerLine);
        //cv::waitKey();
        //int unit = 0;
        //bool counting = false;
        int start = -1;
        // ignore the brink's white points
        while (start < centerLine.cols) {
            start++;
            if (centerLine.at<uchar>(0, start) == BLACK) {
                break;
            }
        }
        centerLine = centerLine.reshape(1, 1);
        std::vector<uchar> array;
        array = centerLine.isContinuous() ? centerLine : centerLine.clone();

        auto units = get_unit_length(array);
        for (const auto &i: units) {
            EanDecoder decoder = EanDecoder(EAN13, i);
            std::cout << decoder.decode(array, start) << std::endl;
        }
        auto temp = find_start_end_patterns(array);
        std::cout << temp.first << std::endl;
    }


}