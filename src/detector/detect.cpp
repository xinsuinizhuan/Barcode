//
// Created by 97659 on 2020/10/14.
//
#include "detector/detect.hpp"


namespace cv {
    using std::vector;

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
        //const double min_side = std::min(src.size().width, src.size().height);
        if (barcode.rows > 512) {
            width = (int) (barcode.cols * (512 * 1.0 / barcode.rows));
            height = 512;

            resize(barcode, resized_barcode, Size(width, height), 0, 0, INTER_AREA);

        } else {
            width = barcode.cols;
            height = barcode.rows;
            resized_barcode = barcode.clone();
        }
        /*if (min_side < 512.0)
        {
            purpose = ZOOMING;
            coeff_expansion = 512.0 / min_side;
            width = cvRound(src.size().width * coeff_expansion);
            height = cvRound(src.size().height * coeff_expansion);
            Size new_size(width, height);
            resize(src, resized_barcode, new_size, 0, 0, INTER_LINEAR);
        }
        else if (min_side > 512.0)
        {
            purpose = SHRINKING;
            coeff_expansion = min_side / 512.0;
            width = cvRound(src.size().width / coeff_expansion);
            height = cvRound(src.size().height / coeff_expansion);
            Size new_size(width, height);
            resize(src, resized_barcode, new_size, 0, 0, INTER_AREA);
        }
        else
        {
            purpose = UNCHANGED;
            coeff_expansion = 1.0;
            width = src.size().width;
            height = src.size().height;
            resized_barcode = barcode.clone();
        }*/
        //resized_barcode.convertTo(resized_barcode, CV_32FC3);
        cvtColor(resized_barcode, gray_barcode, CV_RGBA2GRAY);

    }

    void Detect::localization() {
        findCandidates();   // find areas with low variance in gradient direction
        connectComponents();
        imshow("处理的图", processed_barcode);

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours(processed_barcode, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
        double bounding_rect_area = 0;
        RotatedRect minRect;
        Mat ROI;
        double THRESHOLD_MIN_AREA = height * width * 0.01;
        for (size_t i = 0; i < contours.size(); i++) {
            double area = contourArea(contours[i]);
            minRect = minAreaRect(contours[i]);
            bounding_rect_area = minRect.size.width * minRect.size.height;


            if (bounding_rect_area < THRESHOLD_MIN_AREA) // ignore contour if it is of too small a region
                continue;

            if ((area / bounding_rect_area) > 0.6) // check if contour is of a rectangular object
            {
                Point2f vertices[4];
                minRect.points(vertices);
                double angle = getBarcodeOrientation(contours, i);
                if (angle == USE_ROTATED_RECT_ANGLE) {
                    printf("%f\n", minRect.angle);
                } else {
                    printf("%f %f\n", minRect.angle, angle);

                }
                for (int j = 0; j < 4; j++)
                    line(resized_barcode, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0));
            }
        }
        imshow("框框图", resized_barcode);
        waitKey();
    }

    void Detect::findCandidates() {
        gradient_direction = Mat::zeros(gray_barcode.size(), CV_32F);
        Mat scharr_x(gray_barcode.size(), CV_32F), scharr_y(gray_barcode.size(), CV_32F), variance, mask;
        Scharr(gray_barcode, scharr_x, CV_32F, 1, 0);
        Scharr(gray_barcode, scharr_y, CV_32F, 0, 1);
        phase(scharr_x, scharr_y, gradient_direction, true);

        inRange(gradient_direction, Scalar(180), Scalar(360), mask);
        add(gradient_direction, Scalar(-180), gradient_direction, mask);
        inRange(gradient_direction, Scalar(170), Scalar(180), mask);
        gradient_direction.setTo(Scalar(0), mask);
        gradient_direction.convertTo(gradient_direction, CV_8U);


        // calculate magnitude of gradient, normalize and threshold
        gradient_magnitude = Mat::zeros(gradient_direction.size(), gradient_direction.type());
        magnitude(scharr_x, scharr_y, gradient_magnitude);
        normalize(gradient_magnitude, gradient_magnitude, 0, 255, NormTypes::NORM_MINMAX, CV_8U);
        threshold(gradient_magnitude, gradient_magnitude, 50, 255, THRESH_BINARY | THRESH_OTSU);
        // calculate variances, normalize and threshold so that low-variance areas are bright(255) and
        // high-variance areas are dark(0)
        //clock_t  start = clock();
        Mat raw_variance = calVariance();
        //clock_t  end = clock();
        //printf("%d\n", end-start);

        // replaces every instance of -1 with the max variance
        // this prevents a situation where areas with no edges show up as low variance bec their angles are 0
        // if the value in these cells are set to double.maxval, all the real variances get normalized to 0
        variance = raw_variance.clone();
        double maxVal;
        minMaxLoc(raw_variance, nullptr, &maxVal, nullptr, nullptr);
        mask = Mat::zeros(raw_variance.size(), CV_8U);
        inRange(raw_variance, Scalar(-1), Scalar(-1), mask);
        variance.setTo(Scalar(maxVal), mask);

        normalize(variance, variance, 0, 255, NormTypes::NORM_MINMAX, CV_8U);
        threshold(variance, variance, 75, 255, THRESH_BINARY_INV);

        adjusted_variance = variance.clone();
        inRange(raw_variance, Scalar(-1), Scalar(-1), mask);
        adjusted_variance.setTo(Scalar(127), mask);

        //-5没有设置过，以后再删
        //inRange(raw_variance, Scalar(-5), Scalar(-5), mask);
        //adjusted_variance.setTo(Scalar(63), mask);
        //imshow("梯度大小图", gradient_magnitude);
        //imshow("调整后方差图", adjusted_variance);
        imshow("调整前方差图", variance);

        processed_barcode = variance;
    }

    void Detect::connectComponents() {
        // connect large components by doing morph close followed by morph open
        // use larger element size for erosion to remove small elements joined by dilation
        Mat small_elemSE, large_elemSE;

        small_elemSE = getStructuringElement(MorphShapes::MORPH_ELLIPSE, Size(10, 10));
        large_elemSE = getStructuringElement(MorphShapes::MORPH_ELLIPSE, Size(12, 12));

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
        float sum, sumsq, data;
        integral_gradient_directions = Mat(gray_barcode.size(), CV_32F);
        Mat integral_sumsq(gray_barcode.size(), CV_32F), variance(gray_barcode.size(), CV_32F), gradient_density, temp;

        int width_offset = (int) (0.1 * width / 2);
        int height_offset = (int) (0.1 * height / 2);
        float rect_area;

        // set angle to 0 at all points where gradient magnitude is 0 i.e. where there are no edges
        bitwise_and(gradient_direction, gradient_magnitude, gradient_direction);
        integral(gradient_direction, integral_gradient_directions, integral_sumsq, CV_32F, CV_32F);
        threshold(gradient_magnitude, temp, 1, 1, THRESH_BINARY);
        integral(temp, gradient_density, CV_32F);
        //imshow("非零积分图", gradient_magnitude);
        for (int y = 0; y < height; y++) {
            //pixels_position.clear();
            const uint8_t *gradient_magnitude_row = gradient_magnitude.ptr<uint8_t>(y);

            top_row = ((y - height_offset - 1) < 0) ? -1 : (y - height_offset - 1);
            bottom_row = ((y + height_offset) > height) ? height : (y + height_offset);
            int pos = 0;
            for (; pos < width; pos++) {
                if (gradient_magnitude_row[pos] == 0) {
                    variance.at<float_t>(y,
                                         pos) = -1.0f;// used as marker number in variance grid at points with no gradient
                    continue;
                }
                // then calculate the column locations of the rectangle and set them to -1
                // if they are outside the matrix bounds
                left_col = ((pos - width_offset - 1) < 0) ? -1 : (pos - width_offset - 1);
                right_col = ((pos + width_offset) > width) ? width : (pos + width_offset);
                //we had an integral image to count non-zero elements
                rect_area = calcRectSum(gradient_density, right_col, left_col, top_row, bottom_row);
                // get the values of the rectangle corners from the integral image - 0 if outside bounds
                sum = calcRectSum(integral_gradient_directions, right_col, left_col, top_row, bottom_row);
                sumsq = calcRectSum(integral_sumsq, right_col, left_col, top_row, bottom_row);

                // calculate variance based only on points in the rectangular window which are edges
                // edges are defined as points with high gradient magnitude
                data = (sumsq / rect_area) - (sum * sum / rect_area / rect_area);
                variance.at<float_t>(y, pos) = data;
            }

        }
        return variance;

    }


}

int main(int argc, char **argv) {
    using namespace cv;
    if (argc < 2) {
        Mat srcImage = imread(R"(C:\Users\97659\Pictures\Barcode\TB20.jpg)");

        if (srcImage.empty()) {
            printf("文件不存在");
            exit(1);
        }
        Detect bardet;
        bardet.init(srcImage);
        bardet.localization();


    }
    return 0;
}