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
        vector<RotatedRect> localization_rects;


        vector<RotatedRect> localization_bbox;
        vector<float> bbox_scores;
        vector<int> bbox_indices;
        vector<float> bbox_orientations;

    public:
        void init(const Mat &src);

        void localization();

        vector<RotatedRect> getLocalizationRects();


    protected:
        enum resize_direction {
            ZOOMING, SHRINKING, UNCHANGED
        } purpose = UNCHANGED;
        double coeff_expansion = 1.0;
        int height, width;
        Mat barcode, resized_barcode, gradient_direction, gradient_magnitude, processed_barcode, integral_x_sq, integral_y_sq, integral_xy, integral_edges, consistency, orientation, edge_nums;

        void findCandidates();

        static int compare(const RotatedRect &r1, const RotatedRect &r2) {
            return r1.size.area() > r2.size.area();
        }

        void normalizeRegion(RotatedRect &rect);

        void calConsistency(int window_size);


        inline bool isValidCoord(const Point2f &coord) const;

        static inline double computeOrientation(float y, float x);


        //void locateBarcodes();

        void regionGrowing(int window_size);

    };
}


#endif //BARCODE_DETECT_HPP
