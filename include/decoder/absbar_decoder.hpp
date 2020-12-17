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
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

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
    decodeImg(Mat &mat, const std::vector<std::vector<Point2f>> &pointsArrays) const = 0;

    virtual std::string decodeImg(const Mat &gray, const std::vector<Point2f> &points) const = 0;

    virtual string decodeImg(InputArray img) const = 0;

    virtual ~AbsBarDecoder() = default;

protected:
    virtual string decode(vector<uchar> data, int start) const = 0;

    virtual bool isValid(string result) const = 0;
};

class GuardPatternsNotFindException : Exception
{
public:
    explicit GuardPatternsNotFindException(const std::string &msg) : Exception(0, msg, "", __FILE__, __LINE__)
    {}
};

void cutImage(InputArray _src, OutputArray &_dst, const std::vector<Point2f> &rect);
void fillCounter(const std::vector<uchar> &row, int start, std::vector<int> &counters);

} // namespace cv

#endif //! __OPENCV_BARCODE_ABSBAR_DECODER_HPP__

