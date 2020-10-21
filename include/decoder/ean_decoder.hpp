#ifndef EANDECODER_H
#define EANDECODER_H

#include "absdecoder.hpp"
#include "opencv2/core/mat.hpp"
#include <map>
#include <set>

namespace cv {
//extern struct EncodePair;


    constexpr static int PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << INTEGER_MATH_SHIFT;
    constexpr static int MAX_AVG_VARIANCE = static_cast<int>(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.48f);
    constexpr static int MAX_INDIVIDUAL_VARIANCE = static_cast<int>(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.7f);
    constexpr static uint PATTERN_LENGTH = 4;
    constexpr static uint PART_LENGTH = 6;

    // TODO, let those variables move to their own class
    const static char *EAN13 = "EAN-13";
    const static char *EAN8 = "EAN-8";

    class ean_decoder : public absdecoder {
    public:
        ean_decoder(const char *name);

        ~ean_decoder();

        //输入初始位置固定的2值化后的数据, 输出解码字符串
        std::string decode(std::vector<uchar> data, int start, bool isRevert) const override;
        //Detect encode type
        std::string decode_and_detect(std::vector<uchar> data) const override;

        std::string getName() const override;

    private:
        std::string name;//EAN具体解码类别：EAN-13 / EAN-8
        uchar unitWidth;
        uchar bitsNum;
        uchar codeLength;
        static constexpr uchar EAN13LENGTH = 95;
        //TODO EAN8 Length ...

        bool isValid(std::string result) const override;

        // 传入简化宽度后的bits
        std::string parseCode(std::vector<uchar> part) const;

        //返回编码内容和编码集
        EncodePair getContent(uchar code) const;

        bool delimiterIsValid(std::vector<uchar> data) const;

        static int decodeDigit(const std::vector<uchar> &row, std::vector<int> &counters, int rowOffset,
                               std::vector<std::vector<int>> patterns);

        static std::pair<int, int> find_start_end_patterns(const std::vector<uchar> &row);

        static std::pair<int, int> find_gurad_patterns(const std::vector<uchar> &row,
                                                       int rowOffset,
                                                       uchar whiteFirst,
                                                       const std::vector<int> &pattern,
                                                       std::vector<int> counters);

    };

}
#endif // !EANDECODER_H

