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
#include "opencv2/decoder/ean13_decoder.hpp"
#include "opencv2/decoder/bardecode.hpp"

namespace cv {

void BarDecode::init(const cv::Mat &src, const std::vector <cv::Point2f> &points)
{
    //CV_TRACE_FUNCTION();
    original = src.clone();
    CV_Assert(!points.empty());
    CV_Assert((points.size() % 4) == 0);
    src_points.clear();
    for (int i = 0; i < points.size(); i += 4)
    {
        vector <Point2f> tempMat{points.cbegin() + i, points.cbegin() + i + 4};
        if (contourArea(tempMat) > 0.0)
        {
            src_points.push_back(tempMat);
        }
    }
    CV_Assert(!src_points.empty());
}

bool BarDecode::decodingProcess()
{
    std::unique_ptr <AbsBarDecoder> decoder(new Ean13Decoder());
    result_info = decoder->rectToResults(original, src_points);
    return !result_info.empty();
}

bool BarDecode::decodeMultiplyProcess()
{
    class ParallelBarCodeDecodeProcess : public ParallelLoopBody
    {
    public:
        ParallelBarCodeDecodeProcess(Mat &inarr_, vector <vector<Point2f>> &src_points_,
                                     vector <std::string> &decoded_info_) : inarr(inarr_), src_points(src_points_),
                                                                            decoded_info(decoded_info_)
        {
            for (int i = 0; i < src_points.size(); ++i)
            {
                decoder.push_back(std::unique_ptr<AbsBarDecoder>(new Ean13Decoder()));
            }
        }

        void operator()(const Range &range) const

        CV_OVERRIDE
        {
            CV_Assert(inarr.channels() == 1);
            Mat gray = inarr.clone();
            for (int i = range.start; i < range.end; i++)
            {
                Mat bar_img;
                cutImage(gray, bar_img, src_points[i]);
                if (bar_img.cols < 500)
                {
                    resize(bar_img, bar_img, Size(500, bar_img.rows));
                }
                decoded_info[i] = decoder[i]->rectToResult(bar_img, src_points[i]);
            }
        }

    private:
        Mat &inarr;
        vector <std::string> &decoded_info;
        vector <vector<Point2f>> &src_points;
        vector <std::unique_ptr<AbsBarDecoder>> decoder;
    };
    result_info.clear();
    result_info.resize(src_points.size());
    ParallelBarCodeDecodeProcess parallelDecodeProcess{original, src_points, result_info};
    parallel_for_(Range(0, int(src_points.size())), parallelDecodeProcess);
    return !result_info.empty();
}
}