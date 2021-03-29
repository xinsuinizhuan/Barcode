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
#include "barcodesr.hpp"
#include "opencv2/highgui.hpp"
#include "super_scale.hpp"
namespace cv{
namespace barcode{


void resize(Mat & src, Mat & dst)
{
    barcode::SuperScale scale;
    std::string dir = "D:/Project/Barcode/Repository/opencv_3rdparty-wechat_qrcode/";
    scale.init(dir+"sr.prototxt", dir+"sr.caffemodel");

    if (src.cols < 600)
    {
//        BarcodeSR sr;
//        dst = sr.upsample(src, 4);
        dst = scale.processImageScale(src, 2, true);
        #ifdef CV_DEBUG
        imshow("sr", dst);
        #endif
//        resize(src, dst, Size(600, src.rows), 0, 0, INTER_AREA);
    }
    else
    {
        dst.create(src.size(), src.type());
    }
}

void ostuPreprocess(Mat & src, Mat & dst)
{
    Mat blur;
    GaussianBlur(src, blur, Size(0, 0), 25);
    addWeighted(src, 2, blur, -1, 0, dst);
    dst.convertTo(dst, CV_8UC1, 1, -20);
    resize(src, dst);
    threshold(dst, dst, 155, 255, THRESH_OTSU + THRESH_BINARY);
}

void hybridPreprocess(Mat & src, Mat & dst)
{
    Mat blur;
    GaussianBlur(src, blur, Size(0, 0), 25);
    addWeighted(src, 2, blur, -1, 0, dst);
    dst.convertTo(dst, CV_8UC1, 1, -20);
    resize(src, dst);
    medianBlur(src, dst, 3);
    hybridBinarization(dst, dst);
}

void preprocess(Mat & src, Mat & dst, int mode)
{
    switch (mode)
    {
        case OSTU:
            ostuPreprocess(src, dst);
            break;
        case HYBRID:
            hybridPreprocess(src, dst);
            break;
        default:
            break;
    }
}

}
}

