//
// Created by 97659 on 2020/10/14.
//

#ifndef BARCODE_DETECT_HPP
#define BARCODE_DETECT_HPP

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include "vector"
#include<ctime>

namespace cv {


    class CV_EXPORTS_W BarcodeDetector {
    public:
        CV_WRAP BarcodeDetector();

        ~BarcodeDetector();


        /** @brief Detects QR code in image and returns the quadrangle containing the code.
         @param img grayscale or color (BGR) image containing (or not) QR code.
         @param points Output vector of vertices of the minimum-area quadrangle containing the code.
         */
        CV_WRAP bool detect(InputArray img, CV_OUT std::vector<RotatedRect> &rects, bool debug = false) const;

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
        CV_WRAP bool
        detectAndDecode(InputArray img, CV_OUT std::vector<std::string> &decoded_info,
                        OutputArray rects = noArray()) const;


    };
}


#endif //BARCODE_DETECT_HPP
