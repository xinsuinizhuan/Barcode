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
#include "detector/detect.hpp"


namespace cv {


void Detect::normalizeRegion(RotatedRect &rect)
{
    Point2f start, p, adjust;
    float barcode_orientation = rect.angle + 90;
    if (rect.size.width < rect.size.height)
    {
        barcode_orientation += 90;
    }
    float long_axis = max(rect.size.width, rect.size.height);
    double x_increment = sin(barcode_orientation * 3.1415926 / 180.0);
    double y_increment = cos(barcode_orientation * 3.1415926 / 180.0);
    
    adjust.x = x_increment > 0 ? 1.0 : (x_increment < 0 ? -1.0 : 0);
    adjust.y = y_increment > 0 ? 1.0 : (y_increment < 0 ? -1.0 : 0);
    
    int num_blanks = 0;
    //计算条形码中最长连续条的长度，作为threshold
    int threshold = cvRound(long_axis * 4.0 / 95.0);
    p.y = adjust.y + rect.center.y + (long_axis / 2.0) * y_increment;
    p.x = adjust.x + rect.center.x + (long_axis / 2.0) * x_increment;
    int val;
    while (isValidCoord(p) && (num_blanks < threshold))
    {
        val = consistency.at<uint8_t>(p);
        if (val == 255)
        {
            num_blanks = 0;
        }
        else
        {
            num_blanks++;
        }
        p.x += x_increment;
        p.y += y_increment;
    }
    start.x = p.x;
    start.y = p.y;
    p.x = rect.center.x - (long_axis / 2.0) * x_increment - adjust.x;
    p.y = rect.center.y - (long_axis / 2.0) * y_increment - adjust.y;
    num_blanks = 0;
    while (isValidCoord(p) && (num_blanks < threshold))
    {
        val = consistency.at<uint8_t>(p);
        if (val == 255)
        {
            num_blanks = 0;
        }
        else
        {
            num_blanks++;
        }
        p.x -= x_increment;
        p.y -= y_increment;
    }
    rect.center = (start + p) / 2.0;
    if (long_axis == rect.size.width)
    {
        rect.size.width = norm(p - start);
    }
    else
    {
        rect.size.height = norm(p - start);
    }
    
}

inline bool Detect::isValidCoord(const Point2f &coord) const
{
    if ((coord.x < 0) || (coord.y < 0))
    {
        return false;
    }
    
    if ((coord.x > width - 1.0) || (coord.y > height - 1.0))
    {
        return false;
    }
    
    return true;
}

static float calcRectSum(const Mat &integral, int right_col, int left_col, int top_row, int bottom_row)
{
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

void Detect::init(const Mat &src)
{
    barcode = src.clone();
    const double min_side = std::min(src.size().width, src.size().height);
//        const double max_side = std::max(src.size().width, src.size().height);

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
//        if (max_side < 320.0) {
//            purpose = ZOOMING;
//            coeff_expansion = 320.0 / max_side;
//            width = cvRound(src.size().width * coeff_expansion);
//            height = cvRound(src.size().height * coeff_expansion);
//            Size new_size(width, height);
//            resize(src, resized_barcode, new_size, 0, 0, INTER_LINEAR);
//        } else
    if (min_side > 512.0)
    {
        purpose = SHRINKING;
        coeff_expansion = min_side / 320.0;
        width = cvRound(src.size().width / coeff_expansion);
        height = cvRound(src.size().height / coeff_expansion);
        Size new_size(width, height);
        resize(src, resized_barcode, new_size, 0, 0, INTER_AREA);
    }
    else if (min_side < 512.0)
    {
        purpose = ZOOMING;
        coeff_expansion = 512.0 / min_side;
        width = cvRound(src.size().width * coeff_expansion);
        height = cvRound(src.size().height * coeff_expansion);
        Size new_size(width, height);
        resize(src, resized_barcode, new_size, 0, 0, INTER_LINEAR);
        
    }
    else
    {
        purpose = UNCHANGED;
        coeff_expansion = 1.0;
        width = src.size().width;
        height = src.size().height;
        resized_barcode = barcode.clone();
    }
    //resized_barcode.convertTo(resized_barcode, CV_32FC3);
//        medianBlur(resized_barcode, resized_barcode, 3);

}


void Detect::localization()
{
    localization_rects.clear();
#ifdef CV_DEBUG
    clock_t start = clock();
    imshow("gray image", resized_barcode);
    
    findCandidates();   // find areas with low variance in gradient direction
    clock_t find_time = clock();
    imshow("image before morphing", processed_barcode);
    
    connectComponents();
    clock_t connect_time = clock();
    imshow("image after morphing", processed_barcode);
    
    locateBarcodes();
    clock_t locate_time = clock();
    
    printf("Finding candidates costs %ld ms, connecting components costs %ld ms, locating barcodes costs %ld ms\n",
           find_time - start, connect_time - find_time, locate_time - connect_time);

#else
    findCandidates();   // find areas with low variance in gradient direction
    connectComponents();
    locateBarcodes();
#endif


}

void Detect::locateBarcodes()
{
    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    findContours(processed_barcode, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    double bounding_rect_area;
    RotatedRect minRect;
    double THRESHOLD_MIN_AREA = height * width * 0.005;
    for (auto &contour : contours)
    {
        double area = contourArea(contour);
        if (area < THRESHOLD_MIN_AREA)
        { // ignore contour if it is of too small a region
            continue;
        }
        minRect = minAreaRect(contour);
        bounding_rect_area = minRect.size.width * minRect.size.height;
        if ((area / bounding_rect_area) > 0.66) // check if contour is of a rectangular object
        {

//                double angle = getBarcodeOrientation(contours, i);
//                if (angle == USE_ROTATED_RECT_ANGLE) {
//                    printf("%f\n", minRect.angle);
//                } else {
//                    printf("%f %f\n", minRect.angle, angle);
//                    while(angle>0)
//                        angle-=90;
//                    minRect.angle = angle;
//                }
//                normalizeRegion(minRect);
            if (minRect.size.width > minRect.size.height)
            {
                minRect.size.width = cvRound(minRect.size.width * 103.0 / 95.0);
            }
            else
            { minRect.size.height = cvRound(minRect.size.height * 103.0 / 95.0); }
            
            
            if (purpose == ZOOMING)
            {
                minRect.center.x /= coeff_expansion;
                minRect.center.y /= coeff_expansion;
                minRect.size.height /= coeff_expansion;
                minRect.size.width /= coeff_expansion;
            }
            else if (purpose == SHRINKING)
            {
                minRect.center.x *= coeff_expansion;
                minRect.center.y *= coeff_expansion;
                minRect.size.height *= coeff_expansion;
                minRect.size.width *= coeff_expansion;
            }
            
            localization_rects.push_back(minRect);
            
        }
    }
}


void Detect::findCandidates()
{
    gradient_direction = Mat::zeros(resized_barcode.size(), CV_32F);
    Mat scharr_x(resized_barcode.size(), CV_32F), scharr_y(resized_barcode.size(), CV_32F), temp;
    Scharr(resized_barcode, scharr_x, CV_32F, 1, 0);
    Scharr(resized_barcode, scharr_y, CV_32F, 0, 1);
    // calculate magnitude of gradient, normalize and threshold
    magnitude(scharr_x, scharr_y, gradient_magnitude);
    normalize(gradient_magnitude, gradient_magnitude, 0, 255, NormTypes::NORM_MINMAX, CV_8U);
    adaptiveThreshold(gradient_magnitude, gradient_magnitude, 1, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 9, 0);
    //threshold(gradient_magnitude, gradient_magnitude, 50, 1, THRESH_BINARY);
//        threshold(gradient_magnitude, gradient_magnitude, 48, 1, THRESH_BINARY);
//        gradient_magnitude.convertTo(gradient_magnitude, CV_8U);
    for (int y = 0; y < height; y++)
    {
        //pixels_position.clear();
        auto *x_row = scharr_x.ptr<float_t>(y);
        auto *y_row = scharr_y.ptr<float_t>(y);
        auto *magnitude_row = gradient_magnitude.ptr<uint8_t>(y);
        int pos = 0;
        for (; pos < width; pos++)
        {
            if (magnitude_row[pos] == 0)
            {
                x_row[pos] = 0;
                y_row[pos] = 0;
                continue;
            }
            if (x_row[pos] < 0)
            {
                x_row[pos] *= -1;
                y_row[pos] *= -1;
            }
        }
    }
    
    integral(scharr_x, temp, integral_x_sq, CV_32F, CV_32F);
    integral(scharr_y, temp, integral_y_sq, CV_32F, CV_32F);
    integral(scharr_x.mul(scharr_y), integral_xy, temp, CV_32F, CV_32F);
    
    
    
    // calculate variances, normalize and threshold so that low-variance areas are bright(255) and
    // high-variance areas are dark(0)
    Mat raw_consistency = calConsistency();
#ifdef CV_DEBUG
    imshow("consistency", raw_consistency);
#endif
//        adaptiveThreshold(raw_consistency,consistency,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,0);
    threshold(raw_consistency, consistency, 220, 255, THRESH_BINARY);

//        normalize(consistency, consistency, 0, 255, NormTypes::NORM_MINMAX, CV_8U);

//        threshold(consistency, consistency, 75, 255, THRESH_BINARY);
    
    
    
    processed_barcode = consistency;
}

void Detect::connectComponents()
{
    // connect large components by doing morph close followed by morph open
    // use larger element size for erosion to remove small elements joined by dilation, element size is experimentally determined
    Mat small_elemSE, large_elemSE;
    int small = 8, large = 10;
    small_elemSE = getStructuringElement(MorphShapes::MORPH_ELLIPSE, Size(small, small));
    large_elemSE = getStructuringElement(MorphShapes::MORPH_ELLIPSE, Size(large, large));
    
    dilate(processed_barcode, processed_barcode, small_elemSE);
    erode(processed_barcode, processed_barcode, large_elemSE);
    
    erode(processed_barcode, processed_barcode, small_elemSE);
    dilate(processed_barcode, processed_barcode, large_elemSE);
}

double Detect::getBarcodeOrientation(const vector<vector<Point> > &contours, int i)
{
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
    {
        barcode_orientation = USE_ROTATED_RECT_ANGLE;
    }
    else
    {
        barcode_orientation = barcode_orientation / num_NonZero;
    }
    
    return barcode_orientation;
}

Mat Detect::calConsistency()
{
    /* calculate variance of gradient directions around each pixel
    in the img_details.gradient_directions matrix
    */
    int right_col, left_col, top_row, bottom_row;
    float xy, x_sq, y_sq, d, rect_area;
    Mat raw_consistency(resized_barcode.size(), CV_8U), gradient_density;
    
    int width_offset = cvRound(0.05 * width / 2);
    int height_offset = cvRound(0.05 * height / 2);
    float THRESHOLD_AREA = float(width_offset * height_offset) * 1.6f;
    integral(gradient_magnitude, gradient_density, CV_32F);
    
    
    
    // set angle to 0 at all points where gradient magnitude is 0 i.e. where there are no edges
    
    //imshow("非零积分图", gradient_magnitude);
    for (int y = 0; y < height; y++)
    {
        //pixels_position.clear();
        auto *consistency_row = raw_consistency.ptr<uint8_t>(y);
        
        top_row = ((y - height_offset - 1) < 0) ? -1 : (y - height_offset - 1);
        bottom_row = ((y + height_offset) > height) ? height : (y + height_offset);
        int pos = 0;
        for (; pos < width; pos++)
        {
            
            // then calculate the column locations of the rectangle and set them to -1
            // if they are outside the matrix bounds
            left_col = ((pos - width_offset - 1) < 0) ? -1 : (pos - width_offset - 1);
            right_col = ((pos + width_offset) > width) ? width : (pos + width_offset);
            rect_area = calcRectSum(gradient_density, right_col, left_col, top_row, bottom_row);
            if (rect_area < THRESHOLD_AREA)
            {
                // 有梯度的点占比小于阈值则视为平滑区域
                consistency_row[pos] = 0;
                continue;
            }
            //we had an integral image to count non-zero elements
            x_sq = calcRectSum(integral_x_sq, right_col, left_col, top_row, bottom_row);
            y_sq = calcRectSum(integral_y_sq, right_col, left_col, top_row, bottom_row);
            xy = calcRectSum(integral_xy, right_col, left_col, top_row, bottom_row);

//                if (rect_area < THRESHOLD_AREA) {
//                    // 有梯度的点占比小于阈值则视为平滑区域
//                    consistency_row[pos] = -1.0f;
//                    continue;
//
//                }
            // get the values of the rectangle corners from the integral image - 0 if outside bounds
            d = sqrt((x_sq - y_sq) * (x_sq - y_sq) + 4 * xy * xy) / (x_sq + y_sq);
            consistency_row[pos] = cvRound(d * 255);

//                variance.at<float_t>(y, pos) = data;
        }
        
    }
    return raw_consistency;
    
}
}