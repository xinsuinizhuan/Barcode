//
// Created by 97659 on 2020/10/14.
//

#ifndef BARCODE_DETECT_HPP
#define BARCODE_DETECT_HPP

#include "opencv2/opencv.hpp"
#include <vector>

namespace cv {

    using std::vector;

    class Detect {
    private:
        const int USE_ROTATED_RECT_ANGLE = 361;

    public:
        void init(const Mat &src);

        void localization();

        vector<RotatedRect> getLocalizationRects() { return localization_rects; }


    protected:
        enum resize_direction {
            ZOOMING, SHRINKING, UNCHANGED
        } purpose = UNCHANGED;
        double coeff_expansion = 1.0;
        int height, width;
        Mat barcode, resized_barcode, gradient_direction, gradient_magnitude, processed_barcode, consistency, integral_x_sq, integral_y_sq, integral_xy;
        vector<RotatedRect> localization_rects;

        void findCandidates();


        double getBarcodeOrientation(const vector<vector<Point> > &contours, int i);

        Mat calConsistency();

        void connectComponents();

        inline bool isValidCoord(const Point2f &coord) const;

        void normalizeRegion(RotatedRect &rect);

        void locateBarcodes();
    };
}


#endif //BARCODE_DETECT_HPP
