#ifndef ABSDECODER_H
#define ABSDECODER_H

#include <string>
#include "opencv2/core/mat.hpp"
#include <utility>
#include <vector>
#include "Barcode_Color.h"
/*
	AbsDecoder 是各种识别方式的抽象类，EAN-13/8 Code128 等等都实现它
	在Bardecoder 中动态选择使用哪种解码Decoder
*/
namespace cv {

    void test(); // TODO

    class AbsDecoder {
    public:
        //input 1 row 2-value Mat, return decode string
        virtual std::string decode(std::vector<uchar> bar, int start) const = 0;

        virtual std::string getName() const = 0;

    private:
        virtual bool isValid() const = 0;
    };

    // TODO 改成枚举
    const std::string EAN13 = "EAN-13";
    const std::string EAN8 = "EAN-8";

    int patternMatchVariance(std::vector<int> counters,
                             std::vector<int> pattern,
                             int maxIndividualVariance);

    struct EncodePair {
        std::string content;
        std::string type;
        bool valid = true;

        EncodePair(std::string content, std::string type) {
            this->content = std::move(content);
            this->type = std::move(type);
        }
    };
}

#endif //! ABSDECODER_H
