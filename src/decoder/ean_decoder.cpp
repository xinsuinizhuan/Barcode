#define _USE_MATH_DEFINES

#include "decoder/ean_decoder.hpp"
#include <iostream>
#include <array>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>
// three digit decode method from https://baike.baidu.com/item/EAN-13

namespace cv {
    // default thought that mat is a matrix after binary-transfer.
    /*Input a mat and it's position rect, return the decode result */
    vector<string> ean_decoder::rectToResults(Mat &mat, const vector<RotatedRect> &rects) const {
        CV_Assert(mat.channels() == 1);
        vector<string> will_return;
        Mat gray = mat.clone();
        // assume the maximum proportion of barcode is half of max(width, height), thickest bar is
        // 0.5*max(width,height)/95 * 4
        int length = max(gray.rows, gray.cols);
        int block_size = length / bitsNum * 2 + 1;
        equalizeHist(gray, gray);
        imshow("hist", gray);
        adaptiveThreshold(gray, gray, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, block_size, 1);
        imshow("binary", gray);
        static int constexpr PART = 10;
        will_return.reserve(rects.size());
        for (const auto &rect : rects) {
            will_return.push_back(rectToResult(gray, mat, rect, PART, false));
#ifdef CV_DEBUG
            cv::imshow("circles", gray);
#endif
        }
        return will_return;
    }


    // input image is
    string ean_decoder::rectToResult(const Mat &gray, Mat &mat, const RotatedRect &rect, int PART, int directly) const {
        std::map<std::string, int> result_vote;
        std::string max_result = "ERROR";
        if (std::max(rect.size.height, rect.size.width) < this->bitsNum) {
            return max_result;
        }
        Point2f vertices[4];
        rect.points(vertices);
        double distance1 = cv::norm(vertices[0] - vertices[1]);
        double distance2 = cv::norm(vertices[1] - vertices[2]);
        vector<std::pair<Point2f, Point2f>> begin_and_ends;
        linesFromRect(rect, distance1 > distance2, PART, begin_and_ends);
        if (directly) {
            linesFromRect(rect, distance1 <= distance2, PART, begin_and_ends);
        }
        for (const auto &pairs: begin_and_ends) {
            const Point2f &begin = pairs.first;
            const Point2f &end = pairs.second;
            std::string result = getString(gray, begin, end);
#ifdef CV_DEBUG
            cv::line(mat, begin, end, cv::Scalar(0, 255, 0));
            cv::line(mat, begin, end, Scalar(0, 0, 255), 2);
            cv::circle(mat, begin, 4, Scalar(255, 0, 0), 2);
            cv::circle(mat, end, 4, Scalar(0, 0, 255), 2);
#endif
            int vote_cnt = 0;
            if (result.size() == digitNumber) {
                result_vote[result] += 1;// if not exist, it will automatically create key-value pair
                if (result_vote[result] > vote_cnt) {
                    vote_cnt = result_vote[result];
                    max_result = result;
                }
            }
        }
        return max_result;
    }

    string ean_decoder::getString(const Mat &gray, const Point2f &begin, const Point2f &end) const {
        LineIterator line = LineIterator(gray, begin, end);
        std::vector<uchar> middle;
        middle.reserve(line.count);
        for (int cnt = 0; cnt < line.count; cnt++, line++) {
            middle.push_back(gray.at<uchar>(line.pos()));
        }
        std::string result = decode(middle, 0);
        if (result.size() != digitNumber) {
            result = decode(std::vector<uchar>(middle.crbegin(), middle.crend()), 0);
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
    void ean_decoder::linesFromRect(const RotatedRect &rotatedRect, int angle, int PART,
                                    vector<std::pair<Point2f, Point2f>> &results) const {
        //bottom Left, top Left, top Right, bottom Right.
        Point2f vertices[4];
        rotatedRect.points(vertices);
        auto width_way = 3, height_way = 1;
        if (angle) {
            width_way = 1, height_way = 3;
        }
        const Point2f &step = (vertices[height_way] - vertices[0]) / PART;
        const Point2f &cbegin = vertices[0];
        const Point2f &cend = vertices[width_way];
        for (int i = 0; i < PART; ++i) {
            results.emplace_back(cbegin + i * step, cend + i * step);
        }
        results.emplace_back(cbegin, cend);
//         else if (angle < 90) {
//            Point2f rightMid, leftMid;
//            float length_of_short = img_size.width / std::tan(angle / M_PI);
//            if (length_of_short > img_size.height) {
//                rightMid = vertices[2] + Point2f{0, length_of_short};
//                leftMid = vertices[0] - Point2f{0, length_of_short};
//                Point2f divideWidth = vertices[2] / (PART / 3);
//                Point2f divideHeight = Point2f{0, length_of_short} / (PART / 3);
//                Point2f divideOtherHeight = leftMid / (PART / 3);
//                for (int i = 0; i < PART / 3; i++) {
//                    results.emplace_back(divideWidth * i, vertices[2] + divideHeight * i);
//                    results.emplace_back(divideOtherHeight * i, divideOtherHeight + rightMid);
//                    results.emplace_back(divideHeight * i + leftMid, vertices[0] + divideWidth * i);
//                }
//            } else {
//
//            }
//        }
    }

    const vector<vector<int>> &get_A_or_C_Patterns() {
        static const vector<vector<int>> A_or_C_Patterns = {
                {3, 2, 1, 1}, // 0
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

    const vector<vector<int>> &get_AB_Patterns() {
        static const vector<vector<int>> AB_Patterns = [] {
            auto AB_Patterns_inited = vector<vector<int >>(20, vector<int>(PATTERN_LENGTH, 0));
            std::copy(get_A_or_C_Patterns().cbegin(), get_A_or_C_Patterns().cend(), AB_Patterns_inited.begin());
            //AB pattern is
            int offset = 10;
            for (int i = 0; i < get_A_or_C_Patterns().size(); ++i) {
                for (int j = 0; j < PATTERN_LENGTH; ++j) {
                    AB_Patterns_inited[i + offset][j] = AB_Patterns_inited[i][PATTERN_LENGTH - j - 1];
                }
            }
            return AB_Patterns_inited;
        }();
        return AB_Patterns;
    }


    const vector<int> &BEGIN_PATTERN() {
        // it just need it's 1:1:1(black:white:black)
        static const vector<int> BEGIN_PATTERN_(3, 1);
        return BEGIN_PATTERN_;
    }

    const vector<int> &MIDDLE_PATTERN() {
        // it just need it's 1:1:1:1:1(white:black:white:black:white)
        static const vector<int> MIDDLE_PATTERN_(5, 1);
        return MIDDLE_PATTERN_;
    }

    const std::array<char, 32> &FIRST_CHAR_ARRAY() {
        // use array to simulation a Hashmap,
        // because the data's size is small,
        // use a hashmap or brute-force search 10 times both can not accept
        static const std::array<char, 32> pattern{
                '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
                '\x00', '\x06', '\x00', '\x00', '\x00', '\x09',
                '\x00', '\x08', '\x03', '\x00', '\x00', '\x00',
                '\x00', '\x05', '\x00', '\x07', '\x02', '\x00',
                '\x00', '\x04', '\x01', '\x00', '\x00', '\x00',
                '\x00', '\x00'
        };
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


    /**
     * decode EAN-13
     * @prama: data: the input array,
     * @prama: start, the index of start order, begin at 0, max-value is data.size()-1
     * it scan begin at the data[start]
     */
    // TODO!, need fix the param: stars's usage
    string ean_decoder::decode(vector<uchar> data, int start) const {
        // at least it should have EAN13LENGTH's bits
        // else it can not decode at all
        char decode_result[14]{'\0'};
        if (data.size() - start < bitsNum) {
            return "size wrong";
        }
        vector<int> gurad_counters{0, 0, 0};
        auto whiteFirst = (*data.begin() != WHITE);
        start = findGuardPatterns(data, start, whiteFirst, BEGIN_PATTERN(), gurad_counters).second;
        vector<int> counters = {0, 0, 0, 0};
        int end = data.size();
        uint32_t first_char_bit = 0;
        // [1,6] are left part of EAN, [7,12] are right part, index 0 is calculated by left part
        for (int i = 1; i < 7 && start < end; ++i) {
            int bestMatch = decodeDigit(data, counters, start, get_AB_Patterns());
            if (bestMatch == -1) {
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
        start = findGuardPatterns(data, start, true, MIDDLE_PATTERN(),
                                  vector<int>(MIDDLE_PATTERN().size())).second;
        for (int i = 0; i < 6 && start < end; ++i) {
            int bestMatch = decodeDigit(data, counters, start, get_A_or_C_Patterns());
            if (bestMatch == -1) {
                return "ERROR";
            }
            decode_result[i + 7] = static_cast<char>('0' + bestMatch);
            start = std::accumulate(counters.cbegin(), counters.cend(), start);
        }
        string result = string(decode_result);
        if (!isValid(result)) {
            return "Wrong: " + result.append(string(digitNumber - result.size(), ' '));
        }
        //TODO throw exception
        return result;
    }

    string ean_decoder::decodeDirectly(InputArray img) const {
        auto Mat = img.getMat();
        auto gray = Mat.clone();
        cv::normalize(gray, gray, 0, 255, NormTypes::NORM_MINMAX, CV_8U);
        cv::threshold(gray, gray, 0, 255, THRESH_BINARY | THRESH_OTSU);
#ifdef CV_DEBUG
        cv::imshow("gray", gray);
#endif
        auto rRect = RotatedRect(Point2f(Mat.cols / 2, Mat.rows / 2),
                                 Size2f(Mat.cols, Mat.rows), 0);
        auto result = rectToResult(gray, Mat, rRect, 50, true);
#ifdef CV_DEBUG
        cv::imshow("origin", Mat);
        cv::waitKey();
#endif
        return result;
    }


    std::pair<int, int> ean_decoder::findGuardPatterns(const vector<uchar> &row,
                                                       int rowOffset,
                                                       uchar whiteFirst,
                                                       const vector<int> &pattern,
                                                       vector<int> counters) {
        std::pair<int, int> will_return{rowOffset, -1};
        int patternLength = pattern.size();
        int width = row.size();
        uchar isWhite = whiteFirst ? WHITE : BLACK;
        rowOffset = std::find(row.cbegin() + rowOffset, row.cend(), isWhite) - row.cbegin();
        //rowOffset = whiteFirst ? row.getNextUnset(rowOffset) : row.getNextSet(rowOffset);
        int counterPosition = 0;
        int patternStart = rowOffset;
        for (int x = rowOffset; x < width; x++) {
            if (row[x] == isWhite) {
                counters[counterPosition]++;
            } else {
                if (counterPosition == patternLength - 1) {
                    if (patternMatch(counters, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
                        return std::make_pair(patternStart, x);
                    }
                    patternStart += counters[0] + counters[1];
                    std::copy(counters.begin() + 2, counters.end(), counters.begin());
                    counters[patternLength - 2] = 0;
                    counters[patternLength - 1] = 0;
                    counterPosition--;
                } else {
                    counterPosition++;
                }
                counters[counterPosition] = 1;
                isWhite = (std::numeric_limits<uchar>::max() - isWhite);
            }
        }
        will_return.second = rowOffset;
        return will_return;
    }

    int ean_decoder::decodeDigit(const vector<uchar> &row, vector<int> &counters, int rowOffset,
                                 vector<vector<int>> patterns) {
        fillCounter(row, rowOffset, counters);
        int bestMatch = -1;
        int bestVariance = MAX_AVG_VARIANCE; // worst variance we'll accept
        for (int i = 0; i < patterns.size(); i++) {
            int variance = patternMatch(counters, patterns[i], MAX_INDIVIDUAL_VARIANCE);
            if (variance < bestVariance) {
                bestVariance = variance;
                bestMatch = i;
            }
        }
        return std::max(-1, bestMatch);
        // -1 is dismatch or means error.
    }

    ean_decoder::ean_decoder(EAN name) {
        if (name == EAN::TYPE13) {
            bitsNum = EAN13LENGTH;
            digitNumber = EAN13DIGITNUMBER;
            this->name = "EAN-13";
            //7 module encode a digit
        }
    }

    string ean_decoder::getName() const {
        return this->name;
    }

    bool ean_decoder::isValid(string result) const {
        if (result.size() != this->digitNumber) {
            return false;
        }
        int sum = 0;
        for (int index = result.size() - 2, i = 1; index >= 0; index--, i++) {
            int temp = result[index] - '0';
            sum += (temp + ((i & 1) != 0 ? temp << 1 : 0));
        }
        return (result.back() - '0') == (10 - (sum % 10)) % 10;
    }

}