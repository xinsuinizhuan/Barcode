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
#include "barcode_data.hpp"
#include "patternmatch.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <iostream>
#include <vector>

namespace cv {
//extern struct EncodePair;
using std::string;
using std::vector;

class ean13_decoder : public UPCEANDecoder
{
public:
    ean13_decoder();

    ~ean13_decoder() override = default;

private:
    string decode(vector<uchar> data, int start) const override;

    bool isValid(string result) const override;
};

} // namespace cv
#endif // !__OPENCV_BARCODE_EAN13_DECODER_HPP__