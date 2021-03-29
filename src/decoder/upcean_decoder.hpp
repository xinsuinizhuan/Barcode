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
#ifndef __OPENCV_BARCODE_UPCEAN_DECODER_HPP__
#define __OPENCV_BARCODE_UPCEAN_DECODER_HPP__

#include "abs_decoder.hpp"
#include <opencv2/core.hpp>
#include <map>
#include <utility>
#include <string>

/**
 *   upcean_decoder the abstract basic class for decode formats,
 *   it will have ean13/8,upc_a,upc_e , etc.. class extend this class
*/
namespace cv {
namespace barcode {
using std::string;
using std::vector;

class UPCEANDecoder : public AbsDecoder
{

public:
    ~UPCEANDecoder() override = default;

    std::vector<Result> decodeImg(InputArray bar_img, const std::vector<std::vector<Point2f>> &pointsArrays) const override;

    std::pair<Result, float> decodeImg(InputArray bar_img, const std::vector<Point2f> &points) const override;


protected:
    size_t bits_num;
    size_t digit_number;

    int decodeDigit(const std::vector<uchar> &row, std::vector<int> &counters, int rowOffset,
                    const std::vector<std::vector<int>> &patterns) const;

    static bool
    findGuardPatterns(const std::vector<uchar> &row, int rowOffset, uchar whiteFirst, const std::vector<int> &pattern,
                      std::vector<int> counters, std::pair<int, int> &result);

    static bool findStartGuardPatterns(const std::vector<uchar> &row, std::pair<int, int> &start_range);

    std::pair<Result, float> rectToResult(const Mat &bar_img, const std::vector<Point2f> &points, int PART) const;

    Result decodeLine(const Mat &bar_img, const Point2i &begin, const Point2i &end) const;

    void
    linesFromRect(const Size2i &shape, bool horizontal, int PART, std::vector<std::pair<Point2i, Point2i>> &results) const;

    Result decode(std::vector<uchar> bar, uint start) const override = 0;

    bool isValid(std::string result) const override = 0;

private:
    void drawDebugLine(Mat& debug_img, Point2i begin, Point2i end) const;
};

const std::vector<std::vector<int>> &get_A_or_C_Patterns();

const std::vector<std::vector<int>> &get_AB_Patterns();

const std::vector<int> &BEGIN_PATTERN();

const std::vector<int> &MIDDLE_PATTERN();

const std::array<char, 32> &FIRST_CHAR_ARRAY();

constexpr static uint PATTERN_LENGTH = 4;
constexpr static int MAX_AVG_VARIANCE = static_cast<int>(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.48f);
constexpr static int MAX_INDIVIDUAL_VARIANCE = static_cast<int>(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.7f);

}
} // namespace cv

#endif //!  __OPENCV_BARCODE_UPCEAN_DECODER_HPP__
