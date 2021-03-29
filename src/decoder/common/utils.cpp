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
#include "opencv2/highgui.hpp"

namespace cv{
namespace barcode{

static std::string model_dir = "./";
static std::string super_resolution_prototxt_path = model_dir + "sr.prototxt";
static std::string super_resolution_model_path = model_dir + "sr.caffemodel";

void enhance(Mat & src, Mat & dst)
{
    SuperScale sr;
    CV_Assert(utils::fs::exists(super_resolution_prototxt_path));
    CV_Assert(utils::fs::exists(super_resolution_model_path));
    sr.init(super_resolution_prototxt_path, super_resolution_model_path);
    dst = sr.processImageScale(src, 2, true);
    dst = sr.processImageScale(dst, 2, true);
    #ifdef CV_DEBUG
    imshow("sr", dst);
    #endif
}


void preprocess(Mat & src, Mat & dst, int mode)
{

    Mat blur;
    GaussianBlur(src, blur, Size(0, 0), 25);
    addWeighted(src, 2, blur, -1, 0, dst);
    dst.convertTo(dst, CV_8UC1, 1, -20);
    enhance(dst, dst);
    switch (mode)
    {
        case OSTU:
            threshold(dst, dst, 155, 255, THRESH_OTSU + THRESH_BINARY);
            break;
        case HYBRID:
            hybridBinarization(dst, dst);
            break;
        default:
            break;
    }
}

}
}

