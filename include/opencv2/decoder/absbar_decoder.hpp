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
#ifndef __OPENCV_BARCODE_ABSBAR_DECODER_HPP__
#define __OPENCV_BARCODE_ABSBAR_DECODER_HPP__

#include <iostream>
#include <vector>
#include <map>
#include <opencv2/imgproc.hpp>

namespace cv {
using std::string;
using std::vector;
constexpr static int BLACK = std::numeric_limits<uchar>::min();
// WHITE elemental area is 0xff
constexpr static int WHITE = std::numeric_limits<uchar>::max();
enum class BarcodeFormat
{
    EAN_8, EAN_13, UPC_A, UPC_E, UPC_EAN_EXTENSION
};
struct BarcodeResult
{
    std::string result;
    BarcodeFormat format;
};

class AbsBarDecoder
{
public:
    virtual std::vector<std::string>
    rectToResults(Mat &mat, const std::vector<std::vector<Point2f>> &pointsArrays) const = 0;

    virtual std::string rectToResult(const Mat &gray, const std::vector<Point2f> &points) const = 0;

    //Detect encode type
    virtual string decodeDirectly(InputArray img) const = 0;

    virtual ~AbsBarDecoder() = default;
};

class GuardPatternsNotFindException : Exception
{
public:
    explicit GuardPatternsNotFindException(const std::string &error) : Exception(0, error, "", __FILE__, __LINE__)
    {}
};

void cutImage(InputArray _src, OutputArray &_dst, const std::vector<Point2f> &rect);

} // namespace cv

#endif //! __OPENCV_BARCODE_ABSBAR_DECODER_HPP__

