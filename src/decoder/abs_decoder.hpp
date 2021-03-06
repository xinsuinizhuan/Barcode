// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
// Copyright (c) 2020-2021 darkliang wangberlinT Certseeds

#ifndef __OPENCV_BARCODE_ABS_DECODER_HPP__
#define __OPENCV_BARCODE_ABS_DECODER_HPP__

#include <opencv2/barcode.hpp>

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
    virtual std::pair<Result, float> decodeROI(InputArray bar_img) const = 0;

    virtual ~AbsDecoder() = default;

protected:
    virtual Result decode(vector<uchar> data, uint start) const = 0;

    virtual bool isValid(string result) const = 0;
};

void cropROI(const Mat &_src, Mat &_dst, const std::vector<Point2f> &rect);

void fillCounter(const std::vector<uchar> &row, uint start, std::vector<int> &counters);

constexpr static uint INTEGER_MATH_SHIFT = 8;
constexpr static int PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << INTEGER_MATH_SHIFT;

int patternMatch(std::vector<int> counters, const std::vector<int> &pattern, uint maxIndividual);
}
} // namespace cv

#endif //! __OPENCV_BARCODE_ABS_DECODER_HPP__
