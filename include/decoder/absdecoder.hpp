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
#ifndef __OPENCV_BARCODE_ABSDECODER_H__
#define __OPENCV_BARCODE_ABSDECODER_H__

#include "opencv2/core/mat.hpp"
#include <utility>
#include <vector>
#include <string>
#include "barcode_data.hpp"
#include "patternmatch.hpp"
/**
 *   absdecoder the abstract basic class for decode formats,
 *   it will have ean13/8, Code128 , etc.. class extend this class
*/
namespace cv {

class absdecoder
{
public:
    //input 1 row 2-value Mat, return decode string
    virtual std::string decode(std::vector<uchar> bar, int start) const = 0;

    virtual std::string decodeDirectly(InputArray img) const = 0;

    virtual std::string getName() const = 0;

    virtual ~absdecoder() = default;

private:
    virtual bool isValid(std::string result) const = 0;
};

void fillCounter(const std::vector<uchar> &row, int start, std::vector<int> &counters);

void cutImage(InputArray _src, OutputArray &_dst, RotatedRect rect);

} // namespace cv

#endif //! __OPENCV_BARCODE_ABSDECODER_H__
