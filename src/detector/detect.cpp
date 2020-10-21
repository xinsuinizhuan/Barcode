//
// Created by 97659 on 2020/10/14.
//
#include "detector/detect.hpp"


namespace cv {
    using std::vector;

    static bool checkBarInputImage(InputArray img, Mat &gray) {
        CV_Assert(!img.empty());
        CV_CheckDepthEQ(img.depth(), CV_8U, "");

        if (img.cols() <= 20 || img.rows() <= 20) {
            return false;  // image data is not enough for providing reliable results
        }
        int incn = img.channels();
        CV_Check(incn, incn == 1 || incn == 3 || incn == 4, "");
        if (incn == 3 || incn == 4) {
            cvtColor(img, gray, COLOR_BGR2GRAY);
        } else {
            gray = img.getMat();
        }
        return true;
    }

//    static void updateRectsResult(vector<RotatedRect> &rects_, const vector<RotatedRect> &rects) {
//    }

    class Detect {
    private:
        const int USE_ROTATED_RECT_ANGLE = 361;

    public:
        void init(const Mat &src);

        void localization(bool debug = true);

        vector<RotatedRect> getLocalizationRects() { return localization_rects; }

        Mat getCandidatePicture();


    protected:
        enum resize_direction {
            ZOOMING, SHRINKING, UNCHANGED
        } purpose;
        double coeff_expansion;
        int height, width;
        Mat barcode, resized_barcode, gradient_direction, gradient_magnitude, integral_gradient_directions, processed_barcode;
        vector<RotatedRect> localization_rects;

        void findCandidates();


        double getBarcodeOrientation(const vector<vector<Point> > &contours, int i);

        Mat calVariance();

        void connectComponents();


        void locateBarcodes();
    };

    static float calcRectSum(const Mat &integral, int right_col, int left_col, int top_row, int bottom_row) {
        // calculates sum of values within a rectangle from a given integral image
        // if top_row or left_col are -1, it uses 0 for their value
        // this is useful when one part of the rectangle lies outside the image bounds
        // if the right col or bottom row falls outside the image bounds, pass the max col or max row to this method
        // does NOT perform any bounds checking
        float top_left, top_right, bottom_left, bottom_right;
        float sum;

        bottom_right = integral.at<float>(bottom_row, right_col);
        top_right = (top_row == -1) ? 0 : integral.at<float>(top_row, right_col);
        top_left = (left_col == -1 || top_row == -1) ? 0 : integral.at<float>(top_row, left_col);
        bottom_left = (left_col == -1) ? 0 : integral.at<float>(bottom_row, left_col);

        sum = (bottom_right - bottom_left - top_right + top_left);
        return sum;
    }

    void Detect::init(const Mat &src) {
        barcode = src.clone();
        const double min_side = std::min(src.size().width, src.size().height);
//        if (barcode.rows > 512) {
//            width = (int) (barcode.cols * (512 * 1.0 / barcode.rows));
//            height = 512;
//
//            resize(barcode, resized_barcode, Size(width, height), 0, 0, INTER_AREA);
//            coeff_expansion = barcode.rows / (1.0 * resized_barcode.rows);
//
//        } else {
//            width = barcode.cols;
//            height = barcode.rows;
//            resized_barcode = barcode.clone();
//            coeff_expansion = 1.0;
//        }
//        if (min_side < 512.0)
//        {
//            purpose = ZOOMING;
//            coeff_expansion = 512.0 / min_side;
//            width = cvRound(src.size().width * coeff_expansion);
//            height = cvRound(src.size().height * coeff_expansion);
//            Size new_size(width, height);
//            resize(src, resized_barcode, new_size, 0, 0, INTER_LINEAR);
//        }
        if (min_side > 320.0) {
            purpose = SHRINKING;
            coeff_expansion = min_side / 320.0;
            width = cvRound(src.size().width / coeff_expansion);
            height = cvRound(src.size().height / coeff_expansion);
            Size new_size(width, height);
            resize(src, resized_barcode, new_size, 0, 0, INTER_AREA);
        } else {
            purpose = UNCHANGED;
            coeff_expansion = 1.0;
            width = src.size().width;
            height = src.size().height;
            resized_barcode = barcode.clone();
        }
        //resized_barcode.convertTo(resized_barcode, CV_32FC3);

    }


    void Detect::localization(bool debug) {
        localization_rects.clear();
        if (debug) {
            clock_t start = clock();

            findCandidates();   // find areas with low variance in gradient direction
            clock_t find_time = clock();
            imshow("image before morphing", processed_barcode);

            connectComponents();
            clock_t connect_time = clock();
            imshow("image after morphing", processed_barcode);

            locateBarcodes();
            clock_t locate_time = clock();

            printf("Finding candidates costs %ld ms, connecting components costs %ld ms, locating barcodes costs %ld ms\n",
                   find_time - start, connect_time - find_time,
                   locate_time - connect_time);
        } else {
            findCandidates();   // find areas with low variance in gradient direction
            connectComponents();
            locateBarcodes();
        }


    }

    void Detect::locateBarcodes() {
        std::vector<std::vector<Point> > contours;
        std::vector<Vec4i> hierarchy;
        findContours(processed_barcode, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
        double bounding_rect_area = 0;
        RotatedRect minRect;
        double THRESHOLD_MIN_AREA = height * width * 0.005;
        for (size_t i = 0; i < contours.size(); i++) {
            double area = contourArea(contours[i]);
            if (area < THRESHOLD_MIN_AREA) // ignore contour if it is of too small a region
                continue;
            minRect = minAreaRect(contours[i]);
            bounding_rect_area = minRect.size.width * minRect.size.height;
            if ((area / bounding_rect_area) > 0.6) // check if contour is of a rectangular object
            {

//                double angle = getBarcodeOrientation(contours, i);
//                if (angle == USE_ROTATED_RECT_ANGLE) {
//                    printf("%f\n", minRect.angle);
//                } else {
//                    printf("%f %f\n", minRect.angle, angle);
////                    while(angle>0)
////                        angle-=90;
////                    minRect.angle = angle;
//                }
                minRect.center.x = minRect.center.x * coeff_expansion;
                minRect.center.y = minRect.center.y * coeff_expansion;
                minRect.size.height *= coeff_expansion;
                minRect.size.width *= coeff_expansion;
                localization_rects.push_back(minRect);

            }
        }
    }


    void Detect::findCandidates() {
        gradient_direction = Mat::zeros(resized_barcode.size(), CV_32F);
        Mat scharr_x(resized_barcode.size(), CV_32F), scharr_y(resized_barcode.size(), CV_32F), variance, mask;
        Scharr(resized_barcode, scharr_x, CV_32F, 1, 0);
        Scharr(resized_barcode, scharr_y, CV_32F, 0, 1);
        phase(scharr_x, scharr_y, gradient_direction, true);

        inRange(gradient_direction, Scalar(180), Scalar(360), mask);
        add(gradient_direction, Scalar(-180), gradient_direction, mask);
        inRange(gradient_direction, Scalar(175), Scalar(180), mask);
        gradient_direction.setTo(Scalar(0), mask);
        inRange(gradient_direction, Scalar(0), Scalar(5), mask);
        gradient_direction.setTo(Scalar(0), mask);

        gradient_direction.convertTo(gradient_direction, CV_8U);


        // calculate magnitude of gradient, normalize and threshold
        gradient_magnitude = Mat::zeros(gradient_direction.size(), gradient_direction.type());
        magnitude(scharr_x, scharr_y, gradient_magnitude);
        normalize(gradient_magnitude, gradient_magnitude, 0, 255, NormTypes::NORM_MINMAX, CV_8U);
        threshold(gradient_magnitude, gradient_magnitude, 50, 255, THRESH_BINARY | THRESH_OTSU);
        // calculate variances, normalize and threshold so that low-variance areas are bright(255) and
        // high-variance areas are dark(0)
        Mat raw_variance = calVariance();
        // replaces every instance of -1 with the max variance
        // this prevents a situation where areas with no edges show up as low variance bec their angles are 0
        // if the value in these cells are set to double.maxval, all the real variances get normalized to 0
        variance = raw_variance.clone();
        double maxVal;
        minMaxLoc(raw_variance, nullptr, &maxVal, nullptr, nullptr);
        mask = Mat::zeros(raw_variance.size(), CV_8U);
        inRange(raw_variance, Scalar(-1.5), Scalar(-0.5), mask);
        variance.setTo(Scalar(maxVal), mask);

        normalize(variance, variance, 0, 255, NormTypes::NORM_MINMAX, CV_8U);

        threshold(variance, variance, 75, 255, THRESH_BINARY_INV);

        //adjusted_variance = variance.clone();
        //inRange(raw_variance, Scalar(-1), Scalar(-1), mask);
        //adjusted_variance.setTo(Scalar(127), mask);

        //-5没有设置过，以后再删
        //inRange(raw_variance, Scalar(-5), Scalar(-5), mask);
        //adjusted_variance.setTo(Scalar(63), mask);
        //imshow("梯度大小图", gradient_magnitude);
        //imshow("调整后方差图", adjusted_variance);
        //imshow("variance before adjusted", raw_variance);

        processed_barcode = variance;
    }

    void Detect::connectComponents() {
        // connect large components by doing morph close followed by morph open
        // use larger element size for erosion to remove small elements joined by dilation
        Mat small_elemSE, large_elemSE;
        int small = cvRound(sqrt(width * height) * 0.019), large = cvRound(sqrt(width * height) * 0.023);
        small_elemSE = getStructuringElement(MorphShapes::MORPH_ELLIPSE,
                                             Size(small, small));
        large_elemSE = getStructuringElement(MorphShapes::MORPH_ELLIPSE,
                                             Size(large, large));

        dilate(processed_barcode, processed_barcode, small_elemSE);
        erode(processed_barcode, processed_barcode, large_elemSE);

        erode(processed_barcode, processed_barcode, small_elemSE);
        dilate(processed_barcode, processed_barcode, large_elemSE);
    }

    double Detect::getBarcodeOrientation(const vector<vector<Point> > &contours, int i) {
        // get mean angle within contour region so we can rotate by that amount

        Mat mask = Mat::zeros(processed_barcode.size(), CV_8U);
        Mat temp_directions = Mat::zeros(processed_barcode.size(), CV_8U);
        Mat temp_magnitudes = Mat::zeros(processed_barcode.size(), CV_8U);

        drawContours(mask, contours, i, Scalar(255), -1); // -1 thickness to fill contour
        bitwise_and(gradient_direction, mask, temp_directions);
        bitwise_and(gradient_magnitude, mask, temp_magnitudes);

        // gradient_direction now contains non-zero values only where there is a gradient
        // mask now contains angles only for pixels within region enclosed by contour

        double barcode_orientation = cv::sum(temp_directions)[0];
        int num_NonZero = countNonZero(temp_magnitudes);
        if (num_NonZero == 0)
            barcode_orientation = USE_ROTATED_RECT_ANGLE;
        else
            barcode_orientation = barcode_orientation / num_NonZero;

        return barcode_orientation;
    }

    Mat Detect::calVariance() {
        /* calculate variance of gradient directions around each pixel
        in the img_details.gradient_directions matrix
        */
        int right_col, left_col, top_row, bottom_row;
        float sum, sum_sq, data;
        integral_gradient_directions = Mat(resized_barcode.size(), CV_32F);
        Mat integral_sum_sq(resized_barcode.size(), CV_32F), variance(resized_barcode.size(),
                                                                      CV_32F), gradient_density, temp;

        int width_offset = cvRound(0.05 * width / 2);
        int height_offset = cvRound(0.05 * height / 2);
//        float THRESHOLD_AREA = float(width_offset * height_offset) * 1.2f;
        float rect_area;                    // min number of gradient edges in rectangular window to consider as non-zero


        // set angle to 0 at all points where gradient magnitude is 0 i.e. where there are no edges
        bitwise_and(gradient_direction, gradient_magnitude, gradient_direction);
        integral(gradient_direction, integral_gradient_directions, integral_sum_sq, CV_32F, CV_32F);
        threshold(gradient_magnitude, temp, 1, 1, THRESH_BINARY);
        integral(temp, gradient_density, CV_32F);
        //imshow("非零积分图", gradient_magnitude);
        for (int y = 0; y < height; y++) {
            //pixels_position.clear();
            const uint8_t *gradient_magnitude_row = gradient_magnitude.ptr<uint8_t>(y);
            auto *variance_row = variance.ptr<float_t>(y);

            top_row = ((y - height_offset - 1) < 0) ? -1 : (y - height_offset - 1);
            bottom_row = ((y + height_offset) > height) ? height : (y + height_offset);
            int pos = 0;
            for (; pos < width; pos++) {
                if (gradient_magnitude_row[pos] == 0) {
                    variance_row[pos] = -1.0f;
                    // used as marker number in variance grid at points with no gradient
                    continue;
                }
                // then calculate the column locations of the rectangle and set them to -1
                // if they are outside the matrix bounds
                left_col = ((pos - width_offset - 1) < 0) ? -1 : (pos - width_offset - 1);
                right_col = ((pos + width_offset) > width) ? width : (pos + width_offset);
                //we had an integral image to count non-zero elements
                rect_area = calcRectSum(gradient_density, right_col, left_col, top_row, bottom_row);
//                if (rect_area < THRESHOLD_AREA) {
//                    // 有梯度的点占比小于阈值则视为平滑区域
//                    variance_row[pos] = -1.0f;
//                    continue;
//
//                }
                // get the values of the rectangle corners from the integral image - 0 if outside bounds
                sum = calcRectSum(integral_gradient_directions, right_col, left_col, top_row, bottom_row);
                sum_sq = calcRectSum(integral_sum_sq, right_col, left_col, top_row, bottom_row);

                // calculate variance based only on points in the rectangular window which are edges
                // edges are defined as points with high gradient magnitude,平方的均值减去均值的平方

                data = (sum_sq / rect_area) - (sum * sum / rect_area / rect_area);
                variance_row[pos] = data;

//                variance.at<float_t>(y, pos) = data;
            }

        }
        return variance;

    }

    BarcodeDetector::BarcodeDetector() = default;

    BarcodeDetector::~BarcodeDetector() = default;

    bool BarcodeDetector::detect(InputArray img, CV_OUT std::vector<RotatedRect> &rects) const {
        Mat inarr;
        if (!checkBarInputImage(img, inarr)) {
            return false;
        }

        Detect bardet;
        bardet.init(inarr);
        bardet.localization();
        vector<RotatedRect> _rects = bardet.getLocalizationRects();
        rects.assign(_rects.begin(), _rects.end());
        return true;
    }

    bool BarcodeDetector::decode(InputArray img, const std::vector<RotatedRect> &rects, CV_OUT
                                 vector<std::string> &decoded_info) const {
        Mat inarr;
        if (!checkBarInputImage(img, inarr)) {
            return false;
        }
        CV_Assert(!rects.empty());

        return true;
    }
}

int main(int argc, char **argv) {
    using namespace cv;
    if (argc < 2) {
        VideoCapture capture(0);
        capture.set(CAP_PROP_FRAME_WIDTH, 1920);
        capture.set(CAP_PROP_FRAME_HEIGHT, 1080);
        BarcodeDetector bardet;
        Mat frame;
        clock_t start;
        float fps;
        Point2f vertices[4];
        while (true) {
            start = clock();
            capture.read(frame);
            std::vector<RotatedRect> rects;
            bardet.detect(frame, rects);
            for (auto &rect : rects) {
                rect.points(vertices);
                for (int j = 0; j < 4; j++)
                    line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0));
            }
            fps = 1.0f * CLOCKS_PER_SEC / (float) (clock() - start);
            std::cout << fps << " fps" << std::endl;
            imshow("bounding boxes", frame);
//            for(int i = 0;i<bardet.rotated)
            if (waitKey(1) > 0) break;


        }
    }
    return 0;
}