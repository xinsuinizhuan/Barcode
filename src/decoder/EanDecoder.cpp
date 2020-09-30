#include "decoder/EanDecoder.h"
#include <iostream>
#include <array>
/*	目前只做了正向图片识别，如果图片是反的需要反转一下识别的序列
	TODO：
	1. 单位宽度如果低于4像素并且黑白条的宽度不均匀会存在识别错误，需要优化，如果能达到识别2像素单位的条码就不错了
	2. 错误校验
*/
// 三种编码方式 https://baike.baidu.com/item/EAN-13
namespace cv {
    constexpr static int Pattern_Length = 4;

    constexpr static std::array<std::array<int, Pattern_Length>, 10> A_Patterns{{
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
                                                                                }};// NOLINT

    // right for A
    inline static auto AB_Patterns_init() {
        std::array<std::array<int, Pattern_Length>, 20> AB_Patterns_inited{};
        std::copy(A_Patterns.cbegin(), A_Patterns.cend(), AB_Patterns_inited.begin());
        for (int i = 10; i < 20; ++i) {
            for (int j = 0; j < Pattern_Length; ++j) {
                AB_Patterns_inited[i][j] = AB_Patterns_inited[i - 10][Pattern_Length - j - 1];
            }
        }
        return AB_Patterns_inited;
    }

    const static auto AB_Patterns = AB_Patterns_init();

    const static std::map<uchar, std::string> A = {
            {0x0D, "0"},
            {0x19, "1"},
            {0x13, "2"},
            {0x3D, "3"},
            {0x23, "4"},
            {0x31, "5"},
            {0x2F, "6"},
            {0x3B, "7"},
            {0x37, "8"},
            {0x0B, "9"}
    };

    static std::map<uchar, std::string> B = {
            {0x27, "0"},
            {0x33, "1"},
            {0x1B, "2"},
            {0x21, "3"},
            {0x1D, "4"},
            {0x39, "5"},
            {0x05, "6"},
            {0x11, "7"},
            {0x09, "8"},
            {0x17, "9"}
    };

    static std::map<uchar, std::string> R = {
            {0x72, "0"},
            {0x66, "1"},
            {0x6C, "2"},
            {0x42, "3"},
            {0x5C, "4"},
            {0x4E, "5"},
            {0x50, "6"},
            {0x44, "7"},
            {0x48, "8"},
            {0x74, "9"}
    };

    static std::map<std::string, std::string> prefixMap = {
            {"AAAAAA", "0"},
            {"AABABB", "1"},
            {"AABBAB", "2"},
            {"AABBBA", "3"},
            {"ABAABB", "4"},
            {"ABBAAB", "5"},
            {"ABBBAA", "6"},
            {"ABABAB", "7"},
            {"ABABBA", "8"},
            {"ABBABA", "9"}
    };


    EanDecoder::EanDecoder(const std::string &name, uchar unitWidth) {
        this->name = name;
        this->unitWidth = unitWidth;
        if (name == EAN13) {
            bitsNum = EAN13LENGTH;
            //7 module 编码一个digit
            codeLength = 7;
        }
    }

    EanDecoder::~EanDecoder() = default;

/*
	data 输入初始位置固定的2值化后的数据, 输出解码字符串
	start 第一个条码出现的下标
	return 解码后的条码内容
*/
    std::string EanDecoder::decode(std::vector<uchar> data, int start) const {
        // 首先应该从上到下分成16条来做
        // 然后每个数字是从左向右用分成四个部分,完全可以将其委托了另外一个函数
        //  decode EAN-13
        //检查总长度, 错误直接返回
        if (data.size() - start + 1 < unitWidth * EAN13LENGTH) {
            return "size wrong";
        }
        auto barcodeItr = data.cbegin() + start;
        std::vector<uchar> headDelimiter;
        std::vector<uchar> leftPart;
        std::vector<uchar> middleDelimiter;
        std::vector<uchar> rightPart;
        std::vector<uchar> tailDelimiter;

        // count the temp's length
        int barCnt = 0;
        uchar preBar = *barcodeItr;
        int tempWidth = 0;
        // TODO: there should add a step to preproduce the unit length by left head and tail delimiter
        for (; barcodeItr != data.cend(); ++barcodeItr) {
            if (preBar == *barcodeItr) {
                tempWidth++;
            } else {
                int tempLen = std::round(static_cast<double>(tempWidth) / unitWidth);
                if (tempLen > 4) {
                    tempLen = 4;
                }
                barCnt += tempLen;
                for (int i = 0; i < tempLen; i++) {
                    if (barCnt <= 3) {
                        //head delimiter
                        headDelimiter.push_back(preBar);
                    } else if (barCnt > 3 && barCnt <= 45) {
                        //left part
                        leftPart.push_back(preBar);
                    } else if (barCnt > 45 && barCnt <= 50) {
                        //middle delimiter
                        middleDelimiter.push_back(preBar);
                    } else if (barCnt > 50 && barCnt <= 92) {
                        //right part
                        rightPart.push_back(preBar);
                    } else if (barCnt > 92 && barCnt <= 95) {
                        //tail delimiter
                        tailDelimiter.push_back(preBar);
                    }
                }
                preBar = *barcodeItr;
                tempWidth = 1;
                if (barCnt == 3 && !delimiterIsValid(headDelimiter)) {
                    return "head delimiter wrong";
                }
            }
        }
        std::string result = parseCode(leftPart) + parseCode(rightPart);

        return result;
    }

    std::string EanDecoder::getName() const {
        return this->name;
    }

    bool EanDecoder::isValid() const {
        return false;
    }

// 传入简化宽度后的bits
    std::string EanDecoder::parseCode(std::vector<uchar> part) const {
        std::vector<uchar> array(codeLength);
        std::string content = "";
        for (int i = 0; i < part.size(); i++) {
            int digit = i / codeLength;
            array[digit] = array[digit] << 1;
            if (part[i] == BLACK) {
                array[digit] += 1;
            }
        }
        std::string types = "";
        //TODO 检查result 是否属于A,B,R中的其中一种,如果不属于则反相扫描
        for (int i = 0; i < codeLength; i++) {
            EncodePair temp = getContent(array[i]);
            if (temp.valid) {
                content.append(temp.content);
                types.append(temp.type);
            }
        }
        auto iter = prefixMap.find(types);

        if (iter != prefixMap.end()) {
            content.insert(0, iter->second);
        }
        return content;
    }

//返回编码内容和编码集
    EncodePair EanDecoder::getContent(uchar code) const {
        //std::map<uchar, std::string>::iterator iter;
        auto iter = A.find(code);
        if (iter != A.end()) {
            return EncodePair(iter->second, "A");
        }
        iter = B.find(code);
        if (iter != B.end()) {
            return EncodePair(iter->second, "B");
        }
        iter = R.find(code);
        if (iter != R.end()) {
            return EncodePair(iter->second, "R");
        }
        EncodePair wrong = EncodePair("WRONG", "WRONG");
        wrong.valid = false;
        return wrong;
    }

    bool EanDecoder::delimiterIsValid(std::vector<uchar> data) const {
        //检查头分界符
        constexpr uchar delimiter[3]{BLACK, WHITE, BLACK};
        for (int i = 0; i < 3; i++) {
            if (data[i] != delimiter[i]) {
                return false;
            }
        }
        return true;
    }

    std::set<uchar> get_unit_length(const std::vector<uchar> &data) {
        //cv::waitKey();
        int unit = 0;
        //bool counting = false;
        constexpr int DelimiterLength = 3;
        constexpr uchar Delimiter[DelimiterLength]{BLACK, WHITE, BLACK};
        int start = std::find(data.cbegin(), data.cend(), BLACK) - data.cbegin();
        int end = data.size() - 1 - (std::find(data.crbegin(), data.crend(), BLACK) - data.crbegin());
        std::set<uchar> units;
        for (unsigned char i : Delimiter) {
            unit = 0;
            while (start < data.size()) {
                start++;
                unit++;
                if (data[start] != i) {
                    units.insert(unit);
                    break;
                }
            }
            unit = 0;
            while (end >= 0) {
                end--;
                unit++;
                if (data[end] != i) {
                    units.insert(unit);
                    break;
                }
            }
        }
        return units;
    }

    constexpr static int INTEGER_MATH_SHIFT = 8;
    constexpr static int PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << INTEGER_MATH_SHIFT;
    constexpr static int MAX_AVG_VARIANCE = static_cast<int>(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.48f);
    constexpr static int MAX_INDIVIDUAL_VARIANCE = static_cast<int>(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.7f);

    static std::vector<int> BEGIN_PATTERN{1, 1, 1};

    std::pair<int, int> find_start_end_patterns(const std::vector<uchar> &row) {
        bool foundStart = false;
        std::pair<int, int> startRange{};
        int nextStart = 0;
        std::vector<int> counters{0, 0, 0};
        while (!foundStart) {
            std::fill(std::begin(counters), std::end(counters), 0);
            startRange = find_gurad_patterns(row, nextStart, false, BEGIN_PATTERN, counters);
            int start = startRange.first;
            nextStart = startRange.second;
            // Make sure there is a quiet zone at least as big as the start pattern before the barcode.
            // If this check would run off the left edge of the image, do not accept this barcode,
            // as it is very likely to be a false positive.
            int quietStart = start - (nextStart - start);
            if (quietStart >= 0) {
                // TODO ,后续二值化之后用bitarray,这里要做优化.
                foundStart = true;
                for (int i = quietStart; i < start; i++) {
                    if (row[i] != WHITE) {
                        foundStart = false;
                    }
                }
                //foundStart = row.isRange(quietStart, start, false);
            }
        }
        return startRange;
    }

    static std::pair<int, int> find_gurad_patterns(const std::vector<uchar> &row,
                                                   int rowOffset,
                                                   uchar whiteFirst,
                                                   std::vector<int> &pattern,
                                                   std::vector<int> &counters) {
        int patternLength = pattern.size();
        int width = row.size();
        uchar isWhite = whiteFirst;
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
                    if (patternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
                        return std::make_pair(patternStart, x);
                    }
                    patternStart += counters[0] + counters[1];
                    std::copy(counters.begin() + 2, counters.end(), pattern.begin());
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
        return std::make_pair(-1, -1);
    }
}