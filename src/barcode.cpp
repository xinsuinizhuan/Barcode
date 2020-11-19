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
//
// Created by 97659 on 2020/11/3.
//

#include "precomp.hpp"
#include "barcode.hpp"

namespace cv {
static bool checkBarInputImage(InputArray img, Mat &gray)
{
    CV_Assert(!img.empty());
    CV_CheckDepthEQ(img.depth(), CV_8U, "");
    if (img.cols() <= 20 || img.rows() <= 20)
    {
        return false;  // image data is not enough for providing reliable results
    }
    int incn = img.channels();
    CV_Check(incn, incn == 1 || incn == 3 || incn == 4, "");
    if (incn == 3 || incn == 4)
    {
        cvtColor(img, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = img.getMat();
    }
    return true;
}

struct BarcodeDetector::Impl
{
public:
    Impl() = default;
    
    ~Impl() = default;
};

BarcodeDetector::BarcodeDetector() : p(new Impl)
{}

BarcodeDetector::~BarcodeDetector() = default;

bool BarcodeDetector::detect(InputArray img, CV_OUT std::vector<RotatedRect> &rects) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        return false;
    }
    
    Detect bardet;
    bardet.init(inarr);
    bardet.localization();
    vector<RotatedRect> _rects = bardet.getLocalizationRects();
    rects.assign(_rects.begin(), _rects.end());
    return true;
}

bool BarcodeDetector::decode(InputArray img, const std::vector<RotatedRect> &rects, CV_OUT
                             vector<std::string> &decoded_info) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        return false;
    }
    CV_Assert(!rects.empty());
    ean_decoder decoder(TYPE_EAN13);
    vector<std::string> _decoded_info = decoder.rectToUcharlist(inarr, rects);
    decoded_info.assign(_decoded_info.begin(), _decoded_info.end());
    
    return true;
}

bool BarcodeDetector::detectAndDecode(InputArray img, CV_OUT vector<std::string> &decoded_info, CV_OUT
                                      vector<RotatedRect> &rects) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        return false;
    }
    this->detect(img, rects);
    if (rects.empty())
    {
        return false;
    }
    this->decode(img, rects, decoded_info);
//        Detect bardet;
//        bardet.init(inarr);
//        bardet.localization();
//        vector<RotatedRect> _rects = bardet.getLocalizationRects();
//        rects.assign(_rects.begin(), _rects.end());
//        if (_rects.empty()) {
//            return false;
//        }
//        ean_decoder decoder("");
//
//        vector<std::string> _decoded_info = decoder.rectToUcharlist(inarr, _rects);
//        decoded_info.assign(_decoded_info.begin(), _decoded_info.end());
    return true;
}

}
