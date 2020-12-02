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

#ifndef __OPENCV_BARCODE_HPP__
#define __OPENCV_BARCODE_HPP__

#include "detector/detect.hpp"
#include "decoder/ean_decoder.hpp"
#include "decoder/binaryzation.hpp"

namespace cv {
class CV_EXPORTS_W BarcodeDetector
{
public:
    CV_WRAP BarcodeDetector();

    ~BarcodeDetector();


    /** @brief Detects QR code in image and returns the quadrangle containing the code.
     @param img grayscale or color (BGR) image containing (or not) QR code.
     @param points Output vector of vertices of the minimum-area quadrangle containing the code.
     */
    CV_WRAP bool detect(InputArray img, CV_OUT std::vector<RotatedRect> &rects) const;

    /** @brief Decodes barcode in image once it's found by the detect() method.

     Returns UTF8-encoded output string or empty string if the code cannot be decoded.
     @param img grayscale or color (BGR) image containing bar code.
     @param rects vector of rotated rectangle found by detect() method (or some other algorithm).
     @param decoded_info UTF8-encoded output vector of string or empty vector of string if the codes cannot be decoded.
     */
    CV_WRAP bool decode(InputArray img, const std::vector<RotatedRect> &rects, CV_OUT
                        std::vector<std::string> &decoded_info) const;


    /** @brief Both detects and decodes barcode

     @param img grayscale or color (BGR) image containing QR code.
     @param rects optional output array of vertices of the found QR code quadrangle. Will be empty if not found.
     @param decoded_info UTF8-encoded output vector of string or empty vector of string if the codes cannot be decoded.
     */
    CV_WRAP bool detectAndDecode(InputArray img, CV_OUT std::vector<std::string> &decoded_info, CV_OUT
                                 std::vector<RotatedRect> &rects) const;

    CV_WRAP bool detectDirectly(InputArray img, CV_OUT string &decoded_info) const;

protected:
    struct Impl;
    Ptr<Impl> p;
};
}


#endif //BARCODE_BARCODE_HPP
