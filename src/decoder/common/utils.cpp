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
#include "utils.hpp"
#include "hybrid_binarizer.hpp"

namespace cv{
namespace barcode{


void preprocess(Mat & src, Mat & dst)
{
    Mat blur;
    GaussianBlur(src, blur, Size(0, 0), 25);
    addWeighted(src, 2, blur, -1, 0, dst);
    dst.convertTo(dst, CV_8UC1, 1, -20);
}

Mat binaryzation(Mat src, int mode)
{
    Mat dst;
    switch (mode)
    {
        case OSTU:
            threshold(src, dst, 155, 255, THRESH_OTSU + THRESH_BINARY);
            break;
        case HYBRID:
            hybridBinarization(src, dst);
            break;
        default:
            break;
    }
    return dst;
}
}
}

