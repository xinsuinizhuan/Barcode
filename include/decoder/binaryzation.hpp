#ifndef __OPENCV_BINARYZATION_H__

#include <opencv2/imgproc.hpp>
#include <deque>
#include <opencv2/opencv.hpp>
//test
#include <fstream>
#include <iostream>

namespace cv {
void adaptBinaryzation(cv::InputArray src, cv::OutputArray &dst);

/**
 *
 * @param src source img, which has to be grayscale img
 * @param dst binary img
 * @param window_size used to calculate local threshold which has to be odd number
 * @param alpha (0, 1)
 */
void enhanceLocalBinaryzation(cv::InputArray src, cv::OutputArray &dst, int window_size, float alpha);
}

#endif