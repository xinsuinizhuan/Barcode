//
// Created by 97659 on 2020/11/3.
//

#ifndef BARCODE_BARCODE_HPP
#define BARCODE_BARCODE_HPP

#include "detector/detect.hpp"
#include "decoder/ean_decoder.hpp"
#include "decoder/binaryzation.hpp"

namespace cv {
    class CV_EXPORTS_W BarcodeDetector {
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
        CV_WRAP bool
        detectAndDecode(InputArray img, CV_OUT std::vector<std::string> &decoded_info, CV_OUT
                        std::vector<RotatedRect> &rects) const;

        CV_WRAP bool
        detectDirectly(InputArray img, CV_OUT string &decoded_info, CV_OUT
                       vector<RotatedRect> &rects) const;

    protected:
        struct Impl;
        Ptr<Impl> p;


    };
}


#endif //BARCODE_BARCODE_HPP
