#include "decoder/ean_decoder.hpp"
#include <iostream>
#include <array>
// 三种编码方式 https://baike.baidu.com/item/EAN-13
namespace cv {
    const std::vector<std::vector<int>> &get_A_or_C_Patterns() {
        static const std::vector<std::vector<int>> A_or_C_Patterns = {
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

    const std::vector<std::vector<int>> &get_AB_Patterns() {
        static const std::vector<std::vector<int>> AB_Patterns = [] {
            auto AB_Patterns_inited = std::vector<std::vector<int>>(20, std::vector<int>(PATTERN_LENGTH, 0));
            std::copy(get_A_or_C_Patterns().cbegin(), get_A_or_C_Patterns().cend(), AB_Patterns_inited.begin());
            //AB pattern is
            int offset = 10;
            for (int i = 0; i < get_A_or_C_Patterns().size(); ++i) {
                for (int j = 0; j < PATTERN_LENGTH; ++j) {
                    AB_Patterns_inited[i + offset][j] = AB_Patterns_inited[i][PATTERN_LENGTH-j-1];
                }
            }
            return AB_Patterns_inited;
        }();
        return AB_Patterns;
    }

    ean_decoder::ean_decoder(const char *const name) {
        this->name = std::string(name);
        this->unitWidth = -1;
        if (name == EAN13) {
            bitsNum = EAN13LENGTH;
            //7 module 编码一个digit
            codeLength = 7;
        }
    }

    ean_decoder::~ean_decoder() = default;

    const std::vector<int> &BEGIN_PATTERN() {
        static const std::vector<int> BEGIN_PATTERN_(3, 1);
        return BEGIN_PATTERN_;
    }

    const std::vector<int> &MIDDLE_PATTERN() {
        static const std::vector<int> MIDDLE_PATTERN_(5, 1);
        return MIDDLE_PATTERN_;
    }

    const std::array<char, 32> &FIRST_CHAR_ARRAY() {
        // use array to simulation a Hashmap,
        // becuase the datasize is small,
        // use a hashmap or brute-force search 10 times both can not accept
        static const std::array<char, 32> pattern{
                '\x00', '0', '0', '0', '0', '0',
                '0', '\x06', '0', '0', '0', '\x09',
                '0', '\x08', '\x03', '0', '0', '0',
                '0', '\x05', '0', '\x07', '\x02', '0',
                '0', '\x04', '\x01', '0', '0', '0',
                '0', '0'
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
    /**
     * decode EAN-13
     * @prama: data: the input array,
     * @prama: start, the index of start order, begin at 0, max-value is data.size()-1
     * it scan begin at the data[start]
     */
    std::string ean_decoder::decode(std::vector<uchar> data, int start, bool isRevert) const {
        // at least it should have EAN13LENGTH's bits
        // else it can not decode at all
        char decoderesult[14]{'\0'};
        if (data.size() - start < EAN13LENGTH) {
            return "size wrong";
        }
        std::vector<int> guradCounters{0, 0, 0};
        std::pair<int, int> temp = find_gurad_patterns(data, start, false, BEGIN_PATTERN(), guradCounters);
        start = temp.second;
        std::vector<int> counters = {0, 0, 0, 0};
        int end = data.size();
        uint32_t first_char_bit = 0;
        // [1,6] are left part of EAN13, [7,12] are right part, index 0 is calculated by left part
        for (int i = 1; i < 7 && start < end; ++i) {
            int bestMatch = decodeDigit(data, counters, start, get_AB_Patterns());
            if (bestMatch == -1) {
                if (!isRevert) {
                    std::vector<uchar> cpData;
                    cpData.assign(data.rbegin(),data.rend());
                    return decode(cpData,0,true);
                }
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
                                    std::vector<int>(MIDDLE_PATTERN().size())).second;
        for (int i = 0; i < 6 && start < end; ++i) {
            int bestMatch = decodeDigit(data, counters, start, get_A_or_C_Patterns());
            decoderesult[i + 7] = static_cast<char>('0' + bestMatch);
            start = std::accumulate(counters.cbegin(), counters.cend(), start);
        }
        int sums[2]{0, 0};
        for (int i = 1; i < 12; ++i) {
            sums[i % 2 == 0] += decoderesult[i] - '0';
        }
        std::string result = std::string(decoderesult);
        if(!isValid(result)) {
            if (!isRevert) {
                std::vector<uchar> cpData;
                cpData.assign(data.rbegin(),data.rend());
                return decode(cpData,0,true);
            }
            return "wrong: " + result;
        }
        //TODO throw exception
        return result;
    }

    std::string ean_decoder::decode_and_detect(std::vector<uchar> data) const {
        // TODO
        return "!";
    }


    std::string ean_decoder::getName() const {
        return this->name;
    }

    bool ean_decoder::isValid(std::string result) const {
        int i = 1, sum = 0;
        int checkDigit = result[result.size()-1] - '0';
        for(int index = result.size()-i-1;index >= 0;index--,i++) {
            int temp = result[index] - '0';
            if(i % 2 != 0) {
                temp *= 3;
            }
            sum += temp;
        }
        return checkDigit == (10 - (sum % 10)) % 10;
    }



    std::pair<int, int> ean_decoder::find_start_end_patterns(const std::vector<uchar> &row) {
        bool foundStart = false;
        std::pair<int, int> startRange{};
        int nextStart = 0;
        std::vector<int> counters{0, 0, 0};
        while (!foundStart) {
            std::fill(std::begin(counters), std::end(counters), 0);
            startRange = find_gurad_patterns(row, nextStart, false, BEGIN_PATTERN(), counters);
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

    std::pair<int, int> ean_decoder::find_gurad_patterns(const std::vector<uchar> &row,
                                                         int rowOffset,
                                                         uchar whiteFirst,
                                                         const std::vector<int> &pattern,
                                                         std::vector<int> counters) {
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

    int ean_decoder::decodeDigit(const std::vector<uchar> &row, std::vector<int> &counters, int rowOffset,
                                 std::vector<std::vector<int>> patterns) {
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