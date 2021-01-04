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
#ifndef __OPENCV_BARCODE_ABS_DECODER_HPP__
#define __OPENCV_BARCODE_ABS_DECODER_HPP__

#include <opencv2/imgproc.hpp>
#include <opencv2/barcode.hpp>
#include <numeric>
#include <utility>

namespace cv {
namespace barcode {
using std::string;
using std::vector;
constexpr static int BLACK = std::numeric_limits<uchar>::min();
// WHITE elemental area is 0xff
constexpr static int WHITE = std::numeric_limits<uchar>::max();



struct Result
{
    std::string result;
    BarcodeType format = BarcodeType::NONE;

    Result() = default;

    Result(std::string _result, BarcodeType _format)
    {
        result = std::move(_result);
        format = _format;
    }
};

class AbsDecoder
{
public:
    virtual std::vector<Result> decodeImg(Mat &mat, const std::vector<std::vector<Point2f>> &pointsArrays) const = 0;

    virtual Result decodeImg(const Mat &gray, const std::vector<Point2f> &points) const = 0;

    virtual Result decodeImg(InputArray img) const = 0;

    virtual ~AbsDecoder() = default;

protected:
    virtual Result decode(vector<uchar> data, int start) const = 0;

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

constexpr static uint INTEGER_MATH_SHIFT = 8;
constexpr static int PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << INTEGER_MATH_SHIFT;

int patternMatch(std::vector<int> counters, const std::vector<int> &pattern, uint maxIndividual);

inline int
patternMatchVariance(std::vector<int> counters, const std::vector<int> &pattern, int maxIndividualVariance);
}
} // namespace cv

#endif //! __OPENCV_BARCODE_ABS_DECODER_HPP__

