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
#ifndef __OPENCV_BARCODE_BARDECODE_HPP__
#define __OPENCV_BARCODE_BARDECODE_HPP__

#include "abs_decoder.hpp"
#include "common/super_scale.hpp"
namespace cv {
namespace barcode {
using std::vector;
using std::string;

class BarDecode
{
public:
    void init(const Mat &src, const vector<Point2f> &points, string prototxt_path, string model_path);

    const vector<Result> &getDecodeInformation()
    { return result_info; }

    bool decodingProcess();

    bool decodeMultiplyProcess();

private:
    vector<vector<Point2f>> src_points;
    Mat original;
    vector<Result> result_info;
    std::shared_ptr<SuperScale> sr;
    bool use_nn_sr;
};
}
}
#endif //! __OPENCV_BARCODE_BARDECODE_HPP__