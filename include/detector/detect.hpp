//
// Created by 97659 on 2020/10/14.
//

#ifndef BARCODE_DETECT_HPP
#define BARCODE_DETECT_HPP

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include "vector"
#include<ctime>

namespace cv {
    using std::vector;

    class Detect {
    private:
        Mat srcImage;//原图
        Mat desImage;//预处理过的图
        const int USE_ROTATED_RECT_ANGLE = 361;

    public:
        void init(const Mat &src);

        void localization();

    protected:
        enum resize_direction {
            ZOOMING, SHRINKING, UNCHANGED
        } purpose;
        double eps_vertical, eps_horizontal, coeff_expansion;
        int height, width;
        Mat barcode, resized_barcode, gray_barcode, resized_bin_barcode, straight_barcode, gradient_direction, gradient_magnitude, integral_gradient_directions, adjusted_variance, processed_barcode;

        void findCandidates();

        double getBarcodeOrientation(const vector<vector<Point> > &contours, int i);

        Mat calVariance();

        void connectComponents();

        static float calcRectSum(const Mat &integral, int right_col, int left_col, int top_row, int bottom_row);

    };
}


#endif //BARCODE_DETECT_HPP
