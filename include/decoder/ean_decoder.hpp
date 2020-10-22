#ifndef EANDECODER_H
#define EANDECODER_H

#include "absdecoder.hpp"
#include "opencv2/core/mat.hpp"
#include <map>
#include <set>

namespace cv {
//extern struct EncodePair;
    using std::string;
    using std::vector;
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
        string decode(vector<uchar> data, int start) const override;

        //Detect encode type
        string decode_and_detect(vector<uchar> data) const override;

        string getName() const override;

        string decode_outer(vector<uchar> data);

        vector<string> rect_to_ucharlist(Mat &mat, const std::vector<RotatedRect> &rect);

    private:
        string name;//EAN具体解码类别：EAN-13 / EAN-8
        uchar unitWidth;
        uchar bitsNum;
        uchar codeLength;
        static constexpr uchar EAN13LENGTH = 95;
        //TODO EAN8 Length ...

        bool isValid(string result) const override;

        static int decodeDigit(const vector<uchar> &row, vector<int> &counters, int rowOffset,
                               vector<vector<int>> patterns);

        static std::pair<int, int> find_gurad_patterns(const vector<uchar> &row,
                                                       int rowOffset,
                                                       uchar whiteFirst,
                                                       const vector<int> &pattern,
                                                       vector<int> counters);
    };

}
#endif // !EANDECODER_H

