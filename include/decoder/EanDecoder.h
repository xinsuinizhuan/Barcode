#ifndef EANDECODER_H
#define EANDECODER_H

#include "AbsDecoder.h"
#include "opencv2/core/mat.hpp"
#include <map>

extern struct EncodePair;

class EanDecoder : public AbsDecoder {
public:
    EanDecoder(std::string name, uchar unitWidth);

    ~EanDecoder();

    //输入初始位置固定的2值化后的数据, 输出解码字符串
    std::string decode(std::vector<uchar> data, int start) const;

    std::string getName() const;

private:
    std::string name;//EAN具体解码类别：EAN-13 / EAN-8
    uchar unitWidth;
    uchar bitsNum;
    uchar codeLength;
    static const uchar EAN13LENGTH = 95;
    //TODO EAN8 Length ...

    bool isValid() const;

    // 传入简化宽度后的bits
    std::string parseCode(std::vector<uchar> part) const;

    //返回编码内容和编码集
    EncodePair getContent(uchar code) const;

    bool delimiterIsValid(std::vector<uchar> data) const;

};

#endif // !EANDECODER_H

