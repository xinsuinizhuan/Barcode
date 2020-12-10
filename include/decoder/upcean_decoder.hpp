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

#include "opencv2/core/mat.hpp"
#include "opencv2/core.hpp"
#include <utility>
#include <vector>
#include <string>
#include "barcode_data.hpp"
#include "patternmatch.hpp"
#include "decoder/upcean_decoder.hpp"
/**
 *   upcean_decoder the abstract basic class for decode formats,
 *   it will have ean13/8,upc_a,upc_e , etc.. class extend this class
*/
namespace cv {
using std::vector;
using std::string;
enum class BarcodeFormat
{
    EAN_8, EAN_13, UPC_A, UPC_E, UPC_EAN_EXTENSION
};
struct BarcodeResult
{
    std::string result;
    BarcodeFormat format;
};

class upcean_decoder
{


public:
    //input 1 row 2-value Mat, return decode string

    virtual ~upcean_decoder() = default;

    std::vector<std::string> rectToResults(Mat &mat, const std::vector<std::vector<Point2f>> &pointsArrays) const;

    int decodeDigit(const std::vector<uchar> &row, std::vector<int> &counters, int rowOffset,
                    std::vector<std::vector<int>> patterns) const;

//Detect encode type
    string decodeDirectly(InputArray img) const;

protected:
    uchar bitsNum{};
    uchar digitNumber{};


    static std::pair<int, int>
    findGuardPatterns(const std::vector<uchar> &row, int rowOffset, uchar whiteFirst, const std::vector<int> &pattern,
                      std::vector<int> counters);


    static std::pair<int, int> findStartGuardPatterns(const std::vector<uchar> &row);

    std::string
    rectToResult(const Mat &gray, Mat &mat, const std::vector<Point2f> &points, int PART, int directly) const;

    std::string lineDecodeToString(const Mat &bar_img, const Point2i &begin, const Point2i &end) const;

    void
    linesFromRect(const Size2i &shape, int angle, int PART, std::vector<std::pair<Point2i, Point2i>> &results) const;

private:
    virtual std::string decode(std::vector<uchar> bar, int start) const = 0;

    virtual bool isValid(std::string result) const = 0;
};

const std::vector<std::vector<int>> &get_A_or_C_Patterns();

const std::vector<std::vector<int>> &get_AB_Patterns();

const std::vector<int> &BEGIN_PATTERN();

const std::vector<int> &MIDDLE_PATTERN();

const std::array<char, 32> &FIRST_CHAR_ARRAY();

void fillCounter(const std::vector<uchar> &row, int start, std::vector<int> &counters);

void cutImage(InputArray _src, OutputArray &_dst, const std::vector<Point2f> &rect);

class GuardPatternsNotFindException : Exception
{
public:
    explicit GuardPatternsNotFindException(const std::string &msg) : Exception(0, msg, "", __FILE__, __LINE__)
    {}
};

} // namespace cv

#endif //!  __OPENCV_BARCODE_UPCEAN_DECODER_HPP__
