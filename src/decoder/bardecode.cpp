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
#include "ean13_decoder.hpp"
#include "bardecode.hpp"
#include "common/utils.hpp"
#include "opencv2/core/utils/filesystem.hpp"
namespace cv {
namespace barcode {
void BarDecode::init(const cv::Mat &src, const std::vector<cv::Point2f> &points,
                     const string prototxt_path, const string model_path)
{
    //CV_TRACE_FUNCTION();
    original = src.clone();
    CV_Assert(!points.empty());
    CV_Assert((points.size() % 4) == 0);
    src_points.clear();
    for (size_t i = 0; i < points.size(); i += 4)
    {
        vector<Point2f> tempMat{points.cbegin() + i, points.cbegin() + i + 4};
        if (contourArea(tempMat) > 0.0)
        {
            src_points.push_back(tempMat);
        }
    }
    CV_Assert(!src_points.empty());
    if(!prototxt_path.empty() && !model_path.empty())
    {
        CV_Assert(utils::fs::exists(prototxt_path));
        CV_Assert(utils::fs::exists(model_path));
        sr = std::make_shared<SuperScale>();
        int res = sr -> init(prototxt_path, model_path);
        CV_Assert(res == 0);
        use_nn_sr = true;
    }
    else
    {
        use_nn_sr = false;
    }

}

bool BarDecode::decodingProcess()
{
    std::unique_ptr<AbsDecoder> decoder(new Ean13Decoder);

    result_info = decoder->decodeImg(original, src_points);
    return !result_info.empty();
}

bool BarDecode::decodeMultiplyProcess()
{
    class ParallelBarCodeDecodeProcess : public ParallelLoopBody
    {
    public:
        ParallelBarCodeDecodeProcess(vector<Mat> &bar_imgs_, vector<Result> &decoded_info_)
                : bar_imgs(bar_imgs_), decoded_info(decoded_info_)
        {
            //indicate Decoder
            decoders.push_back(std::shared_ptr<AbsDecoder>(new Ean13Decoder()));
        }

        void operator()(const Range &range) const CV_OVERRIDE
        {
            for (int i = range.start; i < range.end; i++)
            {
                auto res = decoders[0]->decodeImg(bar_imgs[i]);
                decoded_info[i] = res.first;
            }
        }

    private:
        vector<Mat> &bar_imgs;
        vector<Result> &decoded_info;
        vector<std::shared_ptr<AbsDecoder>> decoders;
    };
    result_info.clear();
    result_info.resize(src_points.size());
    //sr and cut img
    vector<Mat> bar_imgs;
    for(auto & corners : src_points)
    {
        Mat bar_img;
        cutImage(original, bar_img, corners);
        preprocess(bar_img, bar_img);
        // scale by 4
        bar_img = sr -> processImageScale(bar_img, 2, use_nn_sr);
        bar_img = sr -> processImageScale(bar_img, 2, use_nn_sr);

        bar_img = binaryzation(bar_img, OSTU);
        bar_imgs.emplace_back(bar_img);
    }
    ParallelBarCodeDecodeProcess parallelDecodeProcess{bar_imgs, result_info};
    parallel_for_(Range(0, int(bar_imgs.size())), parallelDecodeProcess);
    return !result_info.empty();
}
}
}