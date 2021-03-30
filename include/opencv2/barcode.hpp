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

#include "opencv2/core.hpp"
#include <ostream>
namespace cv {
namespace barcode {
enum BarcodeType
{
    NONE, EAN_8, EAN_13, UPC_A, UPC_E, UPC_EAN_EXTENSION
};

static inline std::ostream &operator<<(std::ostream &out, const BarcodeType &barcode_type)
{
    switch (barcode_type)
    {
        case BarcodeType::EAN_8:
            out << "EAN_8";
            break;
        case BarcodeType::EAN_13:
            out << "EAN_13";
            break;
        case BarcodeType::UPC_E:
            out << "UPC_E";
            break;
        case BarcodeType::UPC_A:
            out << "UPC_A";
            break;
        case BarcodeType::UPC_EAN_EXTENSION:
            out << "UPC_EAN_EXTENSION";
            break;
        default:
            out << "NONE";
    }
    return out;
}

class CV_EXPORTS_W BarcodeDetector
{
public:
    /**
     * @brief initialize the BarcodeDetector
     * @param prototxt_path prototxt file path for the super resolution model
     * @param model_path model file path for the super resolution model
     */
    CV_WRAP BarcodeDetector(const std::string prototxt_path = "", const std::string model_path = "");

    ~BarcodeDetector();

    /** @brief Detects Barcode in image and returns the rectangle(s) containing the code.
     *
     * @param img grayscale or color (BGR) image containing (or not) Barcode.
     * @param points Output vector of vector of vertices of the minimum-area rotated rectangle containing the codes.
     * For N detected barcodes, the dimensions of this array should be [N][4].
     * Order of four points in vector< Point2f> is bottomLeft, topLeft, topRight, bottomRight.
     */
    CV_WRAP bool detect(InputArray img, OutputArray points) const;

    /** @brief Decodes barcode in image once it's found by the detect() method.
     * Returns UTF8-encoded output string or empty string if the code cannot be decoded.
     *
     * @param img grayscale or color (BGR) image containing bar code.
     * @param points vector of rotated rectangle vertices found by detect() method (or some other algorithm).
     * For N detected barcodes, the dimensions of this array should be [N][4].
     * Order of four points in vector<Point2f> is bottomLeft, topLeft, topRight, bottomRight.
     * @param decoded_info UTF8-encoded output vector of string or empty vector of string if the codes cannot be decoded.
     * @param decoded_type vector of BarcodeType, specifies the type of these barcodes
     */
    CV_WRAP bool decode(InputArray img, InputArray points, CV_OUT std::vector<std::string> &decoded_info, CV_OUT
                        std::vector<BarcodeType> &decoded_type) const;

    /** @brief Both detects and decodes barcode

     * @param img grayscale or color (BGR) image containing barcode.
     * @param decoded_info UTF8-encoded output vector of string(s) or empty vector of string if the codes cannot be decoded.
     * @param decoded_type vector of BarcodeType, specifies the type of these barcodes
     * @param points_ optional output vector of vertices of the found  barcode rectangle. Will be empty if not found.
     */
    CV_WRAP bool detectAndDecode(InputArray img, CV_OUT std::vector<std::string> &decoded_info, CV_OUT
                                 std::vector<BarcodeType> &decoded_type, OutputArray points = noArray()) const;

protected:
    struct Impl;
    Ptr<Impl> p;
private:

};
}
} // cv::barcode::
#endif //__OPENCV_BARCODE_HPP__