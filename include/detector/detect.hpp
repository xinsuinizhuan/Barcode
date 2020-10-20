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
        const int USE_ROTATED_RECT_ANGLE = 361;

    public:
        void init(const Mat &src);

        void localization();

        Mat getCandidatePicture();


    protected:
        enum resize_direction {
            ZOOMING, SHRINKING, UNCHANGED
        } purpose;
        double eps_vertical, eps_horizontal, coeff_expansion;
        int height, width;
        Mat barcode, resized_barcode, gray_barcode, gradient_direction, gradient_magnitude, integral_gradient_directions, adjusted_variance, processed_barcode;
        vector<RotatedRect> localization_rects;

        void findCandidates();


        double getBarcodeOrientation(const vector<vector<Point> > &contours, int i);

        Mat calVariance();

        void connectComponents();


        void locateBarcodes();
    };
}


#endif //BARCODE_DETECT_HPP
