#include "decoder/ean_decoder.hpp"
#include <iostream>
#include <array>
#include <opencv2/imgproc.hpp>
// 三种编码方式 https://baike.baidu.com/item/EAN-13

/**
 * TODO 1. 多条
 */
namespace cv {

    bool isValidCoordinate(Point2f point, const Mat &mat) {
        //TODO fix <=
        if ((point.x <= 0) || (point.y <= 0))
            return false;

        if ((point.x >= mat.cols - 1) || (point.y >= mat.rows - 1))
            return false;

        return true;
    }

    // default thought that mat is a matrix after binary-transfer.
    vector<string> ean_decoder::rect_to_ucharlist(Mat &mat, const vector<RotatedRect> &rects) {
        Mat grey;
        cvtColor(mat, grey, COLOR_BGR2GRAY);
        vector<string> will_return;
        int PART = 16;
        for (const auto &rect : rects) {
            vector<uchar> middle;
            Point2f begin;
            Point2f end;
            Point2f vertices[4];
            rect.points(vertices);

            double distance1 = cv::norm(vertices[0] - vertices[1]);
            double distance2 = cv::norm(vertices[1] - vertices[2]);
            std::string result;
            for(int i = 1,direction = 1;i <= PART/2;direction = -1* direction) {

                if (distance1 > distance2) {
                    double stepx = (vertices[0].x - vertices[3].x)/PART;
                    double stepy = (vertices[0].y - vertices[3].y)/PART;
                    Point2f step(stepx,stepy);
                    begin = (vertices[0] + vertices[3]) / 2 + step*i * direction;
                    end = (vertices[1] + vertices[2]) / 2 + step * i * direction;
                } else {
                    double stepx = (vertices[0].x - vertices[1].x)/PART;
                    double stepy = (vertices[0].y - vertices[1].y)/PART;
                    Point2f step(stepx,stepy);
                    begin = (vertices[0] + vertices[1]) / 2 + step * i * direction;
                    end = (vertices[2] + vertices[3]) / 2 + step * i * direction;
                }
                LineIterator line = LineIterator(grey, begin, end);
                middle.reserve(line.count);
                for(int i = 0;i < line.count;i ++,line++) {
                    middle.push_back(grey.at<uchar>(line.pos()));
                    cv::circle(mat,line.pos(),1,Scalar(0,0,255),1);
                }
                cv::threshold(middle,middle,0,255,THRESH_BINARY|THRESH_OTSU);
                result = this->decode(middle, 0);
                if (result.size() != 13) {
                    result = this->decode(std::vector<uchar>(middle.crbegin(), middle.crend()), 0);
                }
                //cv::line(mat,begin,end,Scalar(0,0,255),2);
                cv::circle(mat,begin,4,Scalar(255,0,0),2);
                cv::circle(mat,end,4,Scalar(0,0,255),2);
                if (result.size() == 13) {
                    break;
                }
                if(direction == -1){
                    i++;
                }
            }
            will_return.push_back(result);

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

    ean_decoder::ean_decoder(const char *const name) {
        this->name = string(name);
        this->unitWidth = -1;
        if (name == EAN13) {
            bitsNum = EAN13LENGTH;
            //7 module 编码一个digit
            codeLength = 7;
        }
    }

    ean_decoder::~ean_decoder() = default;

    const vector<int> &BEGIN_PATTERN() {
        // it just need it's 1:1:1(black:white:balck)
        static const vector<int> BEGIN_PATTERN_(3, 1);
        return BEGIN_PATTERN_;
    }

    const vector<int> &MIDDLE_PATTERN() {
        // it just need it's 1:1:1:1:1(white:black:white:balck:white)
        static const vector<int> MIDDLE_PATTERN_(5, 1);
        return MIDDLE_PATTERN_;
    }

    const std::array<char, 32> &FIRST_CHAR_ARRAY() {
        // use array to simulation a Hashmap,
        // becuase the datasize is small,
        // use a hashmap or brute-force search 10 times both can not accept
        static const std::array<char, 32> pattern{
                '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
                '\x00', '\x06', '\x00', '\x00', '\x00', '\x09',
                '\x00', '\x08', '\x03', '\x00', '\x00', '\x00',
                '\x00', '\x05', '\x00', '\x07', '\x02', '\x00',
                '\x00', '\x04', '\x01', '\x00', '\x00', '\x00',
                '\x00', '\x00'
        };// length is 32 to ensure the security
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

    string ean_decoder::decode_outer(vector<uchar> data) {
        vector<int> guradCounters{0, 0, 0};
        std::pair<int, int> temp = find_gurad_patterns(data, 0, false, BEGIN_PATTERN(), guradCounters);
        int start = temp.first;
        return decode(data, start);
    }

    /**
     * decode EAN-13
     * @prama: data: the input array,
     * @prama: start, the index of start order, begin at 0, max-value is data.size()-1
     * it scan begin at the data[start]
     */
    string ean_decoder::decode(vector<uchar> data, int start) const {
        // at least it should have EAN13LENGTH's bits
        // else it can not decode at all
        char decoderesult[14]{'\0'};
        if (data.size() - start < EAN13LENGTH) {
            return "size wrong";
        }
        vector<int> guradCounters{0, 0, 0};
        std::pair<int, int> temp = find_gurad_patterns(data, start, false, BEGIN_PATTERN(), guradCounters);
        start = temp.second;
        vector<int> counters = {0, 0, 0, 0};
        int end = data.size();
        uint32_t first_char_bit = 0;
        // [1,6] are left part of EAN13, [7,12] are right part, index 0 is calculated by left part
        for (int i = 1; i < 7 && start < end; ++i) {
            int bestMatch = decodeDigit(data, counters, start, get_AB_Patterns());
            if (bestMatch == -1) {
                return "ERROR";
            }
            decoderesult[i] = static_cast<char>('0' + bestMatch % 10);
            start = std::accumulate(counters.cbegin(), counters.cend(), start);
            first_char_bit |= (bestMatch >= 10) << i;
        }
        decoderesult[0] = FIRST_CHAR_ARRAY()[first_char_bit >> 2] + '0';
        // why there need >> 2?
        // first, the i in for-cycle is begin in 1
        // second, the first i = 1 is always
        start = find_gurad_patterns(data, start, true, MIDDLE_PATTERN(),
                                    vector<int>(MIDDLE_PATTERN().size())).second;
        for (int i = 0; i < 6 && start < end; ++i) {
            int bestMatch = decodeDigit(data, counters, start, get_A_or_C_Patterns());
            if (bestMatch == -1) {
                return "ERROR";
            }
            decoderesult[i + 7] = static_cast<char>('0' + bestMatch);
            start = std::accumulate(counters.cbegin(), counters.cend(), start);
        }
        string result = string(decoderesult);
        if (!isValid(result)) {
            return "wrong: " + result;
        }
        //TODO throw exception
        return result;
    }

    string ean_decoder::decode_and_detect(vector<uchar> data) const {
        // TODO
        return "!";
    }


    string ean_decoder::getName() const {
        return this->name;
    }

    bool ean_decoder::isValid(string result) const {
        int i = 1, sum = 0;
        int checkDigit = result[result.size() - 1] - '0';
        for (int index = result.size() - i - 1; index >= 0; index--, i++) {
            int temp = result[index] - '0';
            if (i % 2 != 0) {
                temp *= 3;
            }
            sum += temp;
        }
        return checkDigit == (10 - (sum % 10)) % 10;
    }



//    std::pair<int, int> ean_decoder::find_start_end_patterns(const vector<uchar> &row) {
//        bool foundStart = false;
//        std::pair<int, int> startRange{};
//        int nextStart = 0;
//        vector<int> counters{0, 0, 0};
//        while (!foundStart) {
//            std::fill(std::begin(counters), std::end(counters), 0);
//            startRange = find_gurad_patterns(row, nextStart, false, BEGIN_PATTERN(), counters);
//            int start = startRange.first;
//            nextStart = startRange.second;
//            // Make sure there is a quiet zone at least as big as the start pattern before the barcode.
//            // If this check would run off the left edge of the image, do not accept this barcode,
//            // as it is very likely to be a false positive.
//            int quietStart = start - (nextStart - start);
//            if (quietStart >= 0) {
//                // TODO ,后续二值化之后用bitarray,这里要做优化.
//                foundStart = true;
//                for (int i = quietStart; i < start; i++) {
//                    if (row[i] != WHITE) {
//                        foundStart = false;
//                    }
//                }
//                //foundStart = row.isRange(quietStart, start, false);
//            }
//        }
//        return startRange;
//    }

    std::pair<int, int> ean_decoder::find_gurad_patterns(const vector<uchar> &row,
                                                         int rowOffset,
                                                         uchar whiteFirst,
                                                         const vector<int> &pattern,
                                                         vector<int> counters) {
        std::pair<int, int> will_return{rowOffset, -1};
        int patternLength = pattern.size();
        int width = row.size();
        uchar isWhite = whiteFirst ? WHITE : BLACK;
        // TODO, deque<bool> 版本?
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
                    //System.arraycopy(counters, 2, counters, 0, patternLength - 2);
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