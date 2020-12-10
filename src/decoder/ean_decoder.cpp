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
#include "decoder/ean_decoder.hpp"
#include <iostream>
#include <array>
#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
// three digit decode method from https://baike.baidu.com/item/EAN-13

namespace cv {
// default thought that mat is a matrix after binary-transfer.
/*Input a mat and it's position rect, return the decode result */
vector<string> ean_decoder::rectToResults(Mat &mat, const vector<vector<Point2f>> &pointsArrays) const
{
    CV_Assert(mat.channels() == 1);
    vector<string> will_return;
    Mat gray = mat.clone();
    constexpr int PART = 16;
    for (const auto &points : pointsArrays)
    {
        Mat bar_img;
        cutImage(gray, bar_img, points);
#if CV_DEBUG
        imshow("raw_bar", bar_img);
#endif
        if (bar_img.cols < 500)
        {
            resize(bar_img, bar_img, Size(500, bar_img.rows));
        }
        string max_result = rectToResult(bar_img, mat, points, PART, false);
        will_return.push_back(max_result);
    }
    return will_return;
}

// input image is
string
ean_decoder::rectToResult(const Mat &bar_img, Mat &mat, const vector<Point2f> &points, int PART, int directly) const
{
    Mat blur;
    GaussianBlur(bar_img, blur, Size(0, 0), 25);
    addWeighted(bar_img, 2, blur, -1, 0, bar_img);
    bar_img.convertTo(bar_img, CV_8UC1, 1, -20);
    imshow("preprocess", bar_img);
    threshold(bar_img, bar_img, 155, 255, THRESH_OTSU + THRESH_BINARY);
#ifdef CV_DEBUG
    imshow("barimg", bar_img);
#endif
    std::map<std::string, int> result_vote;
    int vote_cnt = 0;
    int total_vote = 0;
    std::string max_result = "ERROR";
    auto rect_size_height = cv::norm(points[0] - points[1]);
    auto rect_size_width = cv::norm(points[1] - points[2]);
    if (max(rect_size_height, rect_size_width) < EAN13LENGTH)
    {
        return max_result;
    }
#ifdef CV_DEBUG
    Mat bar_copy = bar_img.clone();
#endif
    vector<std::pair<Point2i, Point2i>> begin_and_ends;
    linesFromRect(Size2i{bar_img.rows, bar_img.cols}, true, PART, begin_and_ends);
    if (directly)
    {
        linesFromRect(Size2i{bar_img.rows, bar_img.cols}, false, PART, begin_and_ends);
    }
    std::string result;
    for (const auto &i: begin_and_ends)
    {
        vector<uchar> middle;
        const auto &begin = i.first;
        const auto &end = i.second;
        result = lineDecodeToString(bar_img, begin, end);
#ifdef CV_DEBUG
        try
        {
            std::pair<int, int> start_p = findStartGuardPatterns(middle);
            cv::circle(bar_copy, cv::Point2f(start_p.second, begin.y), 4, Scalar(0, 0, 0), 2);
        } catch (GuardPatternsNotFindException &e)
        {}
        cv::line(bar_copy, begin, end, cv::Scalar(0, 255, 0));
        //cv::line(mat,begin,end,Scalar(0,0,255),2);
        cv::circle(bar_copy, begin, 6, Scalar(0, 0, 0), 2);
        cv::circle(bar_copy, end, 6, Scalar(0, 0, 0), 2);
        imshow("barscan", bar_copy);
        //cv::waitKey(0);
#endif
        if (result.size() == this->digitNumber)
        {
            total_vote++;
            result_vote[result] += 1;
            if (result_vote[result] > vote_cnt)
            {
                vote_cnt = result_vote[result];
                if ((vote_cnt << 1) > total_vote)
                {
                    max_result = result;
                }
            }
        }
    }
    return max_result;
}

string ean_decoder::lineDecodeToString(const Mat &bar_img, const Point2i &begin, const Point2i &end) const
{
    std::string result;
    std::vector<uchar> middle;
    LineIterator line = LineIterator(bar_img, begin, end);
    middle.reserve(line.count);
    for (int cnt = 0; cnt < line.count; cnt++, line++)
    {
        middle.push_back(bar_img.at<uchar>(line.pos()));
    }
    result = this->decode(middle, 0);
    if (result.size() != digitNumber)
    {
        result = this->decode(std::vector<uchar>(middle.crbegin(), middle.crend()), 0);
    }
    return result;
}

/**@Prama img_size is the graph's size ,
* @Prama angle from [0,180)
* 0 is horizontal
* (0-90) top Left to bottom Right
* 90 vertical
* (90-180) lower left to upper right
* */
void
ean_decoder::linesFromRect(const Size2i &shape, int angle, int PART, vector<std::pair<Point2i, Point2i>> &results) const
{
    auto shapef = Size2f(shape);
    Point2i step = Point2i(shapef.height / PART, 0);
    Point2i cend = Point2i(shapef.height / 2, shapef.width - 1);
    Point2i cbegin = Point2i(shapef.height / 2, shapef.width / 2);

    if (angle)
    {
        step = Point2i(0, shapef.width / PART);
        cbegin = Point2i(0, shapef.width / 2);
        cend = Point2i(shapef.height - 1, shapef.width / 2);
    }
    results.reserve(PART + 1);
    for (int i = 1; i <= PART / 2; ++i)
    {
        results.emplace_back(cbegin + i * step, cend + i * step);
        results.emplace_back(cbegin - i * step, cend - i * step);
    }
    results.emplace_back(cbegin, cend);
}

const vector<vector<int>> &get_A_or_C_Patterns()
{
    static const vector<vector<int>> A_or_C_Patterns = {{3, 2, 1, 1}, // 0
                                                        {2, 2, 2, 1}, // 1
                                                        {2, 1, 2, 2}, // 2
                                                        {1, 4, 1, 1}, // 3
                                                        {1, 1, 3, 2}, // 4
                                                        {1, 2, 3, 1}, // 5
                                                        {1, 1, 1, 4}, // 6
                                                        {1, 3, 1, 2}, // 7
                                                        {1, 2, 1, 3}, // 8
                                                        {3, 1, 1, 2}  // 9
    };
    return A_or_C_Patterns;
}

// right for A

const vector<vector<int>> &get_AB_Patterns()
{
    static const vector<vector<int>> AB_Patterns = [] {
        auto AB_Patterns_inited = vector<vector<int >>(20, vector<int>(PATTERN_LENGTH, 0));
        std::copy(get_A_or_C_Patterns().cbegin(), get_A_or_C_Patterns().cend(), AB_Patterns_inited.begin());
        //AB pattern is
        int offset = 10;
        for (int i = 0; i < get_A_or_C_Patterns().size(); ++i)
        {
            for (int j = 0; j < PATTERN_LENGTH; ++j)
            {
                AB_Patterns_inited[i + offset][j] = AB_Patterns_inited[i][PATTERN_LENGTH - j - 1];
            }
        }
        return AB_Patterns_inited;
    }();
    return AB_Patterns;
}

const vector<int> &BEGIN_PATTERN()
{
    // it just need it's 1:1:1(black:white:black)
    static const vector<int> BEGIN_PATTERN_(3, 1);
    return BEGIN_PATTERN_;
}

const vector<int> &MIDDLE_PATTERN()
{
    // it just need it's 1:1:1:1:1(white:black:white:black:white)
    static const vector<int> MIDDLE_PATTERN_(5, 1);
    return MIDDLE_PATTERN_;
}

const std::array<char, 32> &FIRST_CHAR_ARRAY()
{
    // use array to simulation a Hashmap,
    // because the data's size is small,
    // use a hashmap or brute-force search 10 times both can not accept
    static const std::array<char, 32> pattern{
            '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x06', '\x00', '\x00', '\x00', '\x09', '\x00',
            '\x08', '\x03', '\x00', '\x00', '\x00', '\x00', '\x05', '\x00', '\x07', '\x02', '\x00', '\x00', '\x04',
            '\x01', '\x00', '\x00', '\x00', '\x00', '\x00'};
    // length is 32 to ensure the security
    // 0x00000 -> 0  -> 0
    // 0x11010 -> 26 -> 1
    // 0x10110 -> 22 -> 2
    // 0x01110 -> 14 -> 3
    // 0x11001 -> 25 -> 4
    // 0x10011 -> 19 -> 5
    // 0x00111 -> 7  -> 6
    // 0x10101 -> 21 -> 7
    // 0x01101 -> 13 -> 8
    // 0x01011 -> 11 -> 9
    // delete the 1-13's 2 number's bit,
    // it always be A which do not need to count.
    return pattern;
}

string ean_decoder::decodeOuter(vector<uchar> data)
{
    vector<int> guradCounters{0, 0, 0};
    std::pair<int, int> temp;
    try
    {
        temp = findGuardPatterns(data, 0, false, BEGIN_PATTERN(), guradCounters);
    } catch (GuardPatternsNotFindException &e)
    {
        return "ERROR";
    }
    int start = temp.first;
    return decode(data, start);
}

/**
* decode EAN-13
* @prama: data: the input array,
* @prama: start, the index of start order, begin at 0, max-value is data.size()-1
* it scan begin at the data[start]
*/
// TODO!, need fix the param: stars's usage
string ean_decoder::decode(vector<uchar> data, int start) const
{
    // at least it should have EAN13LENGTH's bits
    // else it can not decode at all
    string result;
    char decode_result[14]{'\0'};
    if (data.size() - start < bitsNum)
    {
        return "size wrong";
    }
    try
    {
        start = findStartGuardPatterns(data).second;
        vector<int> counters = {0, 0, 0, 0};
        int end = data.size();
        uint32_t first_char_bit = 0;
        // [1,6] are left part of EAN, [7,12] are right part, index 0 is calculated by left part
        for (int i = 1; i < 7 && start < end; ++i)
        {
            int bestMatch = decodeDigit(data, counters, start, get_AB_Patterns());
            if (bestMatch == -1)
            {
                return "ERROR";
            }
            decode_result[i] = static_cast<char>('0' + bestMatch % 10);
            start = std::accumulate(counters.cbegin(), counters.cend(), start);
            first_char_bit |= (bestMatch >= 10) << i;
        }
        decode_result[0] = static_cast<char>(FIRST_CHAR_ARRAY()[first_char_bit >> 2] + '0');
        // why there need >> 2?
        // first, the i in for-cycle is begin in 1
        // second, the first i = 1 is always
        start = findGuardPatterns(data, start, true, MIDDLE_PATTERN(), vector<int>(MIDDLE_PATTERN().size())).second;
        for (int i = 0; i < 6 && start < end; ++i)
        {
            int bestMatch = decodeDigit(data, counters, start, get_A_or_C_Patterns());
            if (bestMatch == -1)
            {
                return "ERROR";
            }
            decode_result[i + 7] = static_cast<char>('0' + bestMatch);
            start = std::accumulate(counters.cbegin(), counters.cend(), start);
        }
        findGuardPatterns(data, start, false, BEGIN_PATTERN(), vector<int>(BEGIN_PATTERN().size()));
        result = string(decode_result);
        if (!isValid(result))
        {
            return "Wrong: " + result.append(string(13 - result.size(), ' '));
        }
    } catch (GuardPatternsNotFindException &e)
    {
        return "ERROR";
    }
    return result;
}

string ean_decoder::decodeDirectly(InputArray img) const
{
    auto Mat = img.getMat();
    auto gray = Mat.clone();
    constexpr int PART = 50;
//    auto rRect = RotatedRect(Point2f(Mat.cols / 2, Mat.rows / 2), Size2f(Mat.cols, Mat.rows), 0);
//    Point2f points_array[4];
//    rRect.points(points_array);
    vector<Point2f> real_rect{
            Point2f(0, Mat.rows), Point2f(0, 0), Point2f(Mat.cols, 0), Point2f(Mat.cols, Mat.rows)};
    string result = rectToResult(Mat, gray, real_rect, PART, true);
    return result;
}

std::pair<int, int> ean_decoder::findStartGuardPatterns(const vector<uchar> &row)
{
    bool isfind = false;
    std::pair<int, int> start_range{0, -1};
    int next_start = 0;
    while (!isfind)
    {
        vector<int> gurad_counters{0, 0, 0};
        start_range = findGuardPatterns(row, next_start, BLACK, BEGIN_PATTERN(), gurad_counters);
        int start = start_range.first;
        next_start = start_range.second;
        int quiet_start = max(start - (next_start - start), 0);
        isfind = (quiet_start != start);
        for (int i = quiet_start; i < start; i++)
        {
            if (row[i] == BLACK)
            {
                isfind = false;
            }
        }
    }
    return start_range;
}

std::pair<int, int>
ean_decoder::findGuardPatterns(const vector<uchar> &row, int rowOffset, uchar whiteFirst, const vector<int> &pattern,
                               vector<int> counters)
{
    int patternLength = pattern.size();
    int width = row.size();
    uchar isWhite = whiteFirst ? WHITE : BLACK;
    rowOffset = std::find(row.cbegin() + rowOffset, row.cend(), isWhite) - row.cbegin();
    int counterPosition = 0;
    int patternStart = rowOffset;
    for (int x = rowOffset; x < width; x++)
    {
        if (row[x] == isWhite)
        {
            counters[counterPosition]++;
        }
        else
        {
            if (counterPosition == patternLength - 1)
            {
                if (patternMatch(counters, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE)
                {
                    return std::make_pair(patternStart, x);
                }
                patternStart += counters[0] + counters[1];
                std::copy(counters.begin() + 2, counters.end(), counters.begin());
                counters[patternLength - 2] = 0;
                counters[patternLength - 1] = 0;
                counterPosition--;
            }
            else
            {
                counterPosition++;
            }
            counters[counterPosition] = 1;
            isWhite = (std::numeric_limits<uchar>::max() - isWhite);
        }
    }
    throw cv::GuardPatternsNotFindException("pattern not find");
}

int
ean_decoder::decodeDigit(const vector<uchar> &row, vector<int> &counters, int rowOffset, vector<vector<int>> patterns)
{
    fillCounter(row, rowOffset, counters);
    int bestMatch = -1;
    int bestVariance = MAX_AVG_VARIANCE; // worst variance we'll accept
    for (int i = 0; i < patterns.size(); i++)
    {
        int variance = patternMatch(counters, patterns[i], MAX_INDIVIDUAL_VARIANCE);
        if (variance < bestVariance)
        {
            bestVariance = variance;
            bestMatch = i;
        }
    }
    return std::max(-1, bestMatch);
    // -1 is dismatch or means error.
}

ean_decoder::ean_decoder(EAN name)
{
    if (name == EAN::TYPE13)
    {
        bitsNum = EAN13LENGTH;
        digitNumber = EAN13DIGITNUMBER;
        this->name = "EAN-13";
        //7 module encode a digit
    }
}

string ean_decoder::getName() const
{
    return this->name;
}

bool ean_decoder::isValid(string result) const
{
    if (result.size() != this->digitNumber)
    {
        return false;
    }
    int sum = 0;
    for (int index = result.size() - 2, i = 1; index >= 0; index--, i++)
    {
        int temp = result[index] - '0';
        sum += (temp + ((i & 1) != 0 ? temp << 1 : 0));
    }
    return (result.back() - '0') == (10 - (sum % 10)) % 10;
}
}