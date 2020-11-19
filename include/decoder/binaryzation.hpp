/*
Copyright 2020 ${ALL COMMITTERS}

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef __OPENCV_BARCODE_BINARYZATION_HPP__
#define __OPENCV_BARCODE_BINARYZATION_HPP__
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

#endif //__OPENCV_BARCODE_BINARYZATION_HPP__