//
// Created by 97659 on 2020/10/14.
//

#ifndef BARCODE_DETECT_HPP
#define BARCODE_DETECT_HPP
#define PI 3.1415926535897932

#include "opencv2/opencv.hpp"
#include <utility>
#include <vector>

namespace cv {

    using std::vector;
    using std::pair;

    class Detect {
    private:
        const int USE_ROTATED_RECT_ANGLE = 361;
        vector<Rect> localization_rects;


        vector<Rect> localization_bbox;
        vector<float> bbox_scores;
        vector<int> bbox_indices;
        vector<float> bbox_orientations;

    public:
        void init(const Mat &src);

        void localization();

        vector<Rect> getLocalizationRects();


    protected:
        enum resize_direction {
            ZOOMING, SHRINKING, UNCHANGED
        } purpose = UNCHANGED;
        double coeff_expansion = 1.0;
        int height, width;
        Mat barcode, resized_barcode, gradient_direction, gradient_magnitude, processed_barcode, integral_x_sq, integral_y_sq, integral_xy, integral_edges;

        void findCandidates();


        double getBarcodeOrientation(const vector<vector<Point> > &contours, int i);

        Mat calConsistency(Mat &raw_consistency, Mat &orientation, int window_size);

        void connectComponents();

        inline bool isValidCoord(const Point2f &coord) const;

        inline double computeOrientation(float y, float x);

        void normalizeRegion(RotatedRect &rect);

        //void locateBarcodes();

        void regionGrowing(Mat &consistency, Mat &orientation, int window_size);

    };
}


#endif //BARCODE_DETECT_HPP
