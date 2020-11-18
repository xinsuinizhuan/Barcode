#ifndef EANDECODER_H
#define EANDECODER_H

#include "absdecoder.hpp"
#include "opencv2/core/mat.hpp"

namespace cv {
    //extern struct EncodePair;
    using std::string;
    using std::vector;

    // TODO, let those variables move to their own class
    const static char *TYPE_EAN13 = "EAN-13";
    const static char *TYPE_EAN8 = "EAN-8";

    class ean_decoder : public absdecoder {
    public:
        explicit ean_decoder(const char *name);

        ~ean_decoder() override = default;

        //输入初始位置固定的2值化后的数据, 输出解码字符串
        string decode(vector<uchar> data, int start) const override;

        //Detect encode type
        string decodeDirectly(vector<uchar> data) const override;

        string getName() const override;

        vector<string> rectToUcharlist(Mat &mat, const std::vector<RotatedRect> &rects) const;

    private:
        string name; //EAN具体解码类别：EAN-13 / EAN-8
        static constexpr uchar EAN13LENGTH = 95;
        static constexpr uchar EAN13DIGITNUMBER = 13;
        //TODO EAN8 Length ...
        uchar bitsNum;
        uchar digitNumber;

        bool isValid(string result) const override;

        static int decodeDigit(const vector<uchar> &row, vector<int> &counters, int rowOffset,
                               vector<vector<int>> patterns);

        static std::pair<int, int> findGuardPatterns(const vector<uchar> &row,
                                                     int rowOffset,
                                                     uchar whiteFirst,
                                                     const vector<int> &pattern,
                                                     vector<int> counters);
    };

    Mat grayNomalization(Mat mat, double M0, double VAR0);

    vector<Mat> getBarcodeImgs(Mat gray_img, const vector<RotatedRect> &rects);
} // namespace cv
#endif // !EANDECODER_H
