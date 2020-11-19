#include "decoder/ean_decoder.hpp"
#include <iostream>
#include <array>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
// three digit decode method from https://baike.baidu.com/item/EAN-13

namespace cv {
    // default thought that mat is a matrix after binary-transfer.
    /*Input a mat and it's position rect, return the decode result */
    vector<string> ean_decoder::rectToUcharlist(Mat &mat, const vector<RotatedRect> &rects, int PART) const {
        CV_Assert(mat.channels() == 1);
        vector<string> will_return;
        Mat gray = mat.clone();
        // assume the maximum proportion of barcode is half of max(width, height), thickest bar is
        // 0.5*max(width,height)/95 * 4
        int length = max(gray.rows, gray.cols);
        int block_size = length / digitNumber * 2 + 1;
        equalizeHist(gray, gray);
        imshow("hist", gray);
        adaptiveThreshold(gray, gray, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, block_size, 1);
        imshow("binary", gray);
        //constexpr int PART = 10;
        for (const auto &rect : rects) {
            std::map<std::string, int> result_vote;
            std::string max_result = "ERROR";
            if (max(rect.size.height, rect.size.width) < EAN13LENGTH) {
                will_return.push_back(max_result);
                continue;
            }
            Point2f begin, end, vertices[4], cbegin, cend, step;
            rect.points(vertices);
            std::string result;
            double distance1 = cv::norm(vertices[0] - vertices[1]);
            double distance2 = cv::norm(vertices[1] - vertices[2]);
            if (distance1 > distance2) {
                step = (vertices[0] - vertices[3]) / PART;
                cbegin = (vertices[0] + vertices[3]) / 2;
                cend = (vertices[1] + vertices[2]) / 2;
            } else {
                step = (vertices[0] - vertices[1]) / PART;
                cbegin = (vertices[0] + vertices[1]) / 2;
                cend = (vertices[2] + vertices[3]) / 2;
            }
            for (int i = 1, direction = 1; i <= PART / 2; direction = -1 * direction) {
                vector<uchar> middle;
                begin = cbegin + step * i * direction;
                end = cend + step * i * direction;
                LineIterator line = LineIterator(gray, begin, end);
                middle.reserve(line.count);
                for (int cnt = 0; cnt < line.count; cnt++, line++) {
                    middle.push_back(gray.at<uchar>(line.pos()));
                }
                result = this->decode(middle, 0);
                if (result.size() != bitsNum) {
                    result = this->decode(std::vector<uchar>(middle.crbegin(), middle.crend()), 0);
                }
#ifdef CV_DEBUG
                //cv::line(mat, begin, end, cv::Scalar(0, 255, 0));
                //cv::line(mat,begin,end,Scalar(0,0,255),2);
                //cv::circle(mat, begin, 4, Scalar(255, 0, 0), 2);
                //cv::circle(mat, end, 4, Scalar(0, 0, 255), 2);
#endif
                int vote_cnt = 0;
                if (result.size() == bitsNum) {
                    result_vote[result] += 1;// if not exist, it will automatically create key-value pair
                    if (result_vote[result] > vote_cnt) {
                        vote_cnt = result_vote[result];
                        max_result = result;
                    }
                }
                if (direction == -1) {
                    i++;
                }
            }
            will_return.push_back(max_result);
#ifdef CV_DEBUG
            cv::imshow("circles",gray);
#endif
        }
        return will_return;
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
            auto AB_Patterns_inited = vector<vector<int>>(20, vector<int>(PATTERN_LENGTH, 0));
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

    ean_decoder::ean_decoder(EAN name) {
        if (name == EAN::TYPE13) {
            bitsNum = EAN13LENGTH;
            digitNumber = EAN13DIGITNUMBER;
            this->name = "EAN-13";
            //7 module encode a digit
        }
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
        if (data.size() - start < EAN13LENGTH) {
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
            return "Wrong: " + result.append(string(bitsNum - result.size(), ' '));
        }
        //TODO throw exception
        return result;
    }

    string ean_decoder::decodeDirectly(InputArray _img) const {
        // TODO
        auto Mat = _img.getMat();
        auto rRect = RotatedRect(Point2f(Mat.cols / 2, Mat.rows / 2), Size2f(Mat.cols, Mat.rows), 0);
        auto result = rectToUcharlist(Mat, vector<RotatedRect>{rRect}, 30);
        if (!result.empty()) {
            return result.front();
        }
        return "";
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
}