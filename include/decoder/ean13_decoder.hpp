/*
Copyright 2020 ${ALL COMMITTERS}

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef __OPENCV_BARCODE_EAN13_DECODER_HPP__
#define __OPENCV_BARCODE_EAN13_DECODER_HPP__

#include "decoder/upcean_decoder.hpp"
#include "opencv2/core/mat.hpp"

namespace cv {
//extern struct EncodePair;
using std::string;
using std::vector;

class ean13_decoder : public upcean_decoder
{
public:
    ean13_decoder();

    ~ean13_decoder() override = default;

    //输入初始位置固定的2值化后的数据, 输出解码字符串

private:
    string decode(vector<uchar> data, int start) const override;

    bool isValid(string result) const override;

};
} // namespace cv
#endif // !__OPENCV_BARCODE_EAN13_DECODER_HPP__