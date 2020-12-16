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
// Created by 97659 on 2020/10/14.
//
#ifndef __OPENCV_BARCODE_DETECT_HPP__
#define __OPENCV_BARCODE_DETECT_HPP__

#include "opencv2/opencv.hpp"
#include <utility>
#include <vector>

namespace cv {

using std::vector;
using std::pair;

class Detect
{
private:
    vector<RotatedRect> localization_rects;
    vector<RotatedRect> localization_bbox;
    vector<float> bbox_scores;
    vector<int> bbox_indices;
    vector<float> bbox_orientations;
    vector<vector<Point2f>> transformation_points;

public:
    void init(const Mat &src);

    void localization();

    vector<vector<Point2f>> getTransformationPoints()
    { return transformation_points; }

    bool computeTransformationPoints();

protected:
    enum resize_direction
    {
        ZOOMING, SHRINKING, UNCHANGED
    } purpose = UNCHANGED;
    double coeff_expansion = 1.0;
    int height, width;
    Mat barcode, resized_barcode, gradient_magnitude, integral_x_sq, integral_y_sq, integral_xy, integral_edges, consistency, orientation, edge_nums;
    // diagonal, skew_diagonal, horizontal, vertical
    #ifdef CV_DEBUG
    Mat debug_img, debug_proposals;
    #endif
    void preprocess();

    static inline int compare(const RotatedRect &r1, const RotatedRect &r2)
    {
        return r1.size.area() > r2.size.area();
    }

//        void normalizeRegion(RotatedRect &rect);

    void calConsistency(int window_size);


    static inline bool isValidCoord(const Point &coord, const Size &limit);

    static inline double computeOrientation(float y, float x);


    //void locateBarcodes();

    void regionGrowing(int window_size);

    void barcodeErode();

};
}


#endif //__OPENCV_BARCODE_DETECT_HPP__

