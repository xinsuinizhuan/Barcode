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
vector<string> ean_decoder::rectToResults(Mat &mat, const vector<RotatedRect> &rects) const
{
    CV_Assert(mat.channels() == 1);
    vector<string> will_return;
    Mat gray = mat.clone();
//        equalizeHist(gray,gray);
#if CV_DEBUG
    imshow("hist", gray);
#endif
    constexpr int PART = 16;
    for (const auto &rect : rects)
    {
        Mat bar_img;
        cutImage(gray, bar_img, rect);
#if CV_DEBUG
        imshow("raw_bar", bar_img);
#endif
        if (bar_img.cols < 300)
        {
            resize(bar_img, bar_img, Size(300, bar_img.rows));
        }
//        int blocksize = (bar_img.cols / 95) * 4 + 1;
        //imshow("rawbar", bar_img);
        threshold(bar_img, bar_img, 155, 255, THRESH_OTSU + THRESH_BINARY);
//        adaptiveThreshold(bar_img, bar_img, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, blocksize, 1);
        imshow("barimg", bar_img);
        std::map<std::string, int> result_vote;
        int vote_cnt = 0;
        float total_vote = 0;
        std::string max_result = "ERROR";
        if (max(rect.size.height, rect.size.width) < EAN13LENGTH)
        {
            will_return.push_back(max_result);
            continue;
        }
#ifdef CV_DEBUG
        Mat bar_copy = bar_img.clone();
#endif
        Point2i begin;
        Point2i end;
        std::string result;
        for (int i = 1, direction = 1; i <= PART / 2; direction = -1 * direction)
        {
            vector<uchar> middle;
            Point2i step(0, bar_img.rows / PART);
            begin = Point2i(0, bar_img.rows / 2) + step * i * direction;
            end = Point2i(bar_img.cols - 1, bar_img.rows / 2) + step * i * direction;
            LineIterator line = LineIterator(bar_img, begin, end);
            middle.reserve(line.count);
            for (int cnt = 0; cnt < line.count; cnt++, line++)
            {
                middle.push_back(bar_img.at<uchar>(line.pos()));
            }
            result = this->decode(middle, 0);
            if (result.size() != 13)
            {
                result = this->decode(std::vector<uchar>(middle.crbegin(), middle.crend()), 0);
            }
#ifdef CV_DEBUG
            
            cv::line(bar_copy, begin, end, cv::Scalar(0, 255, 0));
            //cv::line(mat,begin,end,Scalar(0,0,255),2);
            cv::circle(bar_copy, begin, 4, Scalar(255, 0, 0), 2);
            cv::circle(bar_copy, end, 4, Scalar(0, 0, 255), 2);
            imshow("barscan", bar_copy);
#endif
            if (result.size() == 13)
            {
                total_vote++;
                if (result_vote.find(result) == result_vote.end())
                {
                    result_vote.insert(std::pair<std::string, int>(result, 1));
                }
                else
                {
                    result_vote[result] += 1;
                }
                if (result_vote[result] > vote_cnt)
                {
                    vote_cnt = result_vote[result];
                    if (vote_cnt / total_vote > 0.5)
                    {
                        max_result = result;
                    }
                }
            }
            if (direction == -1)
            {
                i++;
            }
        }
        will_return.push_back(max_result);
    }
    return will_return;
}

// input image is
string ean_decoder::rectToResult(const Mat &bar_img, Mat &mat, const RotatedRect &rect, int PART, int directly) const
{
    std::map<std::string, int> result_vote;
    std::string max_result = "ERROR";
    if (std::max(rect.size.height, rect.size.width) < this->bitsNum)
    {
        return max_result;
    }
#ifdef CV_DEBUG
    Mat bar_copy = bar_img.clone();
#endif
    Point2f vertices[4];
    rect.points(vertices);
    double distance1 = cv::norm(vertices[0] - vertices[1]);
    double distance2 = cv::norm(vertices[1] - vertices[2]);
    vector<std::pair<Point2f, Point2f>> begin_and_ends;
    linesFromRect(Size2i{bar_img.rows, bar_img.cols}, distance1 > distance2, PART, begin_and_ends);
    if (directly)
    {
        linesFromRect(Size2i{bar_img.rows, bar_img.cols}, distance1 <= distance2, PART, begin_and_ends);
    }
    for (const auto &pairs : begin_and_ends)
    {
        const Point2f &begin = pairs.first;
        const Point2f &end = pairs.second;
        std::string result = lineDecodeToString(bar_img, begin, end);
#ifdef CV_DEBUG
        cv::line(mat, begin, end, cv::Scalar(0, 255, 0));
        cv::line(mat, begin, end, Scalar(0, 0, 255), 2);
        cv::circle(mat, begin, 4, Scalar(255, 0, 0), 2);
        cv::circle(mat, end, 4, Scalar(0, 0, 255), 2);
#endif
        int vote_cnt = 0;
        if (result.size() == digitNumber)
        {
            result_vote[result] += 1; // if not exist, it will automatically create key-value pair
            if (result_vote[result] > vote_cnt)
            {
                vote_cnt = result_vote[result];
                max_result = result;
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
ean_decoder::linesFromRect(const Size2i &shape, int angle, int PART, vector<std::pair<Point2f, Point2f>> &results) const
{
    //bottom Left, top Left, top Right, bottom Right.
    auto shapef = Size2f(shape);
    Point2f step{0, shapef.width / PART};
    const Point2f cbegin{0, 0};
    Point2f cend{shapef.height - 1, 0};
    if (angle)
    {
        step = {shapef.height / PART, 0};
        cend = {0, shapef.width - 1};
    }
    for (int i = 0; i < PART; ++i)
    {
        results.emplace_back(cbegin + i * step, cend + i * step);
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
        auto AB_Patterns_inited = vector<vector<int>>(20, vector<int>(PATTERN_LENGTH, 0));
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
    std::pair<int, int> temp = findGuardPatterns(data, 0, false, BEGIN_PATTERN(), guradCounters);
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
    char decode_result[14]{'\0'};
    if (data.size() - start < bitsNum)
    {
        return "size wrong";
    }
    vector<int> gurad_counters{0, 0, 0};
    start = findGuardPatterns(data, start, false, BEGIN_PATTERN(), gurad_counters).second;
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
    string result = string(decode_result);
    if (!isValid(result))
    {
        return "Wrong: " + result.append(string(13 - result.size(), ' '));
    }
    //TODO throw exception
    return result;
}

string ean_decoder::decodeDirectly(InputArray img) const
{
    auto Mat = img.getMat();
    auto gray = Mat.clone();
    cv::normalize(gray, gray, 0, 255, NormTypes::NORM_MINMAX, CV_8U);
    cv::threshold(gray, gray, 0, 255, THRESH_BINARY | THRESH_OTSU);
#ifdef CV_DEBUG
    cv::imshow("gray", gray);
#endif
    auto rRect = RotatedRect(Point2f(Mat.cols / 2, Mat.rows / 2), Size2f(Mat.cols, Mat.rows), 0);
    auto result = rectToResult(gray, Mat, rRect, 50, true);
#ifdef CV_DEBUG
    cv::imshow("origin", Mat);
    cv::waitKey();
#endif
    return result;
}

std::pair<int, int>
ean_decoder::findGuardPatterns(const vector<uchar> &row, int rowOffset, uchar whiteFirst, const vector<int> &pattern,
                               vector<int> counters)
{
    std::pair<int, int> will_return{rowOffset, -1};
    int patternLength = pattern.size();
    int width = row.size();
    uchar isWhite = whiteFirst ? WHITE : BLACK;
    rowOffset = std::find(row.cbegin() + rowOffset, row.cend(), isWhite) - row.cbegin();
    //rowOffset = whiteFirst ? row.getNextUnset(rowOffset) : row.getNextSet(rowOffset);
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
    will_return.second = rowOffset;
    return will_return;
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
} // namespace cv