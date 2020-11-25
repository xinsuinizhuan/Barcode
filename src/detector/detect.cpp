//
// Created by 97659 on 2020/10/14.
//
#include "detector/detect.hpp"


namespace cv {


    void Detect::normalizeRegion(RotatedRect &rect) {
//        Point2f start, p, adjust;
//        float barcode_orientation = rect.angle + 90;
//        if (rect.size.width < rect.size.height)
//            barcode_orientation += 90;
//        float long_axis = max(rect.size.width, rect.size.height);
//        double x_increment = sin(barcode_orientation * PI / 180.0);
//        double y_increment = cos(barcode_orientation * PI / 180.0);
//
//        adjust.x = x_increment > 0 ? 1.0 : (x_increment < 0 ? -1.0 : 0);
//        adjust.y = y_increment > 0 ? 1.0 : (y_increment < 0 ? -1.0 : 0);
//
//        int num_blanks = 0;
//        //计算条形码中最长连续条的长度，作为threshold
//        int threshold = cvRound(long_axis * 4.0 / 95.0);
//        p.y = adjust.y + rect.center.y + (long_axis / 2.0) * y_increment;
//        p.x = adjust.x + rect.center.x + (long_axis / 2.0) * x_increment;
//        int val;
//        while (isValidCoord(p) && (num_blanks < threshold)) {
//            val = consistency.at<uint8_t>(p);
//            if (val == 255)
//                num_blanks = 0;
//            else
//                num_blanks++;
//            p.x += x_increment;
//            p.y += y_increment;
//        }
//        start.x = p.x;
//        start.y = p.y;
//        p.x = rect.center.x - (long_axis / 2.0) * x_increment - adjust.x;
//        p.y = rect.center.y - (long_axis / 2.0) * y_increment - adjust.y;
//        num_blanks = 0;
//        while (isValidCoord(p) && (num_blanks < threshold)) {
//            val = consistency.at<uint8_t>(p);
//            if (val == 255)
//                num_blanks = 0;
//            else
//                num_blanks++;
//            p.x -= x_increment;
//            p.y -= y_increment;
//        }
//        rect.center = (start + p) / 2.0;
//        if (long_axis == rect.size.width)
//            rect.size.width = norm(p - start);
//        else
//            rect.size.height = norm(p - start);

    }

    inline bool Detect::isValidCoord(const Point2f &coord) const {
        if ((coord.x < 0) || (coord.y < 0))
            return false;

        if ((coord.x > width - 1.0) || (coord.y > height - 1.0))
            return false;

        return true;
    }

    inline double Detect::computeOrientation(float y, float x) {
        if (x >= 0) {
            return atan(y / x) / 2.0;
        }
        if (y >= 0) {
            return (atan(y / x) + PI) / 2.0;
        }
        return (atan(y / x) - PI) / 2.0;
    }

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
//            purpose = ZOOMING;
//            coeff_expansion = 320.0 / max_side;
//            width = cvRound(src.size().width * coeff_expansion);
//            height = cvRound(src.size().height * coeff_expansion);
//            Size new_size(width, height);
//            resize(src, resized_barcode, new_size, 0, 0, INTER_LINEAR);
//        } else
        if (min_side > 1024.0) {
            purpose = SHRINKING;
            coeff_expansion = min_side / 1024.0;
            width = cvRound(src.size().width / coeff_expansion);
            height = cvRound(src.size().height / coeff_expansion);
            Size new_size(width, height);
            resize(src, resized_barcode, new_size, 0, 0, INTER_AREA);
        }
//        else if (min_side < 512.0) {
//            purpose = ZOOMING;
//            coeff_expansion = 512.0 / min_side;
//            width = cvRound(src.size().width * coeff_expansion);
//            height = cvRound(src.size().height * coeff_expansion);
//            Size new_size(width, height);
//            resize(src, resized_barcode, new_size, 0, 0, INTER_LINEAR);
//
//        }
        else {
            purpose = UNCHANGED;
            coeff_expansion = 1.0;
            width = src.size().width;
            height = src.size().height;
            resized_barcode = barcode.clone();
        }
        //resized_barcode.convertTo(resized_barcode, CV_32FC3);
//        medianBlur(resized_barcode, resized_barcode, 3);

    }


    void Detect::localization() {
        localization_bbox.clear();
        bbox_indices.clear();
        bbox_scores.clear();
#ifdef CV_DEBUG
        clock_t start = clock();
//        imshow("gray image", resized_barcode);

        findCandidates();   // find areas with low variance in gradient direction
        clock_t find_time = clock();
        imshow("image", processed_barcode);

//        connectComponents();


        dnn::NMSBoxes(localization_bbox, bbox_scores, 0.95, 0.2, bbox_indices);

        clock_t locate_time = clock();

        printf("Finding candidates costs %ld ms, locating barcodes costs %ld ms\n",
               find_time - start,
               locate_time - find_time);

#else
        findCandidates();   // find areas with low variance in gradient direction
        dnn::NMSBoxes(localization_bbox, bbox_scores, 0.8, 0.2, bbox_indices);
#endif


    }

    vector<Rect> Detect::getLocalizationRects() {
        localization_rects.clear();

        for (std::_Vector_iterator<std::_Vector_val<std::_Simple_types<int>>>::value_type &bbox_index : bbox_indices) {
            localization_rects.push_back(localization_bbox[bbox_index]);
        }
        return localization_rects;
    }

//    void Detect::locateBarcodes() {
//        std::vector<std::vector<Point> > contours;
//        std::vector<Vec4i> hierarchy;
//        findContours(processed_barcode, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
//        double bounding_rect_area;
//        RotatedRect minRect;
//        double THRESHOLD_MIN_AREA = height * width * 0.005;
//        for (auto &contour : contours) {
//            double area = contourArea(contour);
//            if (area < THRESHOLD_MIN_AREA) // ignore contour if it is of too small a region
//                continue;
//            minRect = minAreaRect(contour);
//            bounding_rect_area = minRect.size.width * minRect.size.height;
//            if ((area / bounding_rect_area) > 0.66) // check if contour is of a rectangular object
//            {
//
////                double angle = getBarcodeOrientation(contours, i);
////                if (angle == USE_ROTATED_RECT_ANGLE) {
////                    printf("%f\n", minRect.angle);
////                } else {
////                    printf("%f %f\n", minRect.angle, angle);
////                    while(angle>0)
////                        angle-=90;
////                    minRect.angle = angle;
////                }
////                normalizeRegion(minRect);
//                if (minRect.size.width > minRect.size.height)
//                    minRect.size.width = cvRound(minRect.size.width * 103.0 / 95.0);
//                else minRect.size.height = cvRound(minRect.size.height * 103.0 / 95.0);
//
//
//                if (purpose == ZOOMING) {
//                    minRect.center.x /= coeff_expansion;
//                    minRect.center.y /= coeff_expansion;
//                    minRect.size.height /= coeff_expansion;
//                    minRect.size.width /= coeff_expansion;
//                } else if (purpose == SHRINKING) {
//                    minRect.center.x *= coeff_expansion;
//                    minRect.center.y *= coeff_expansion;
//                    minRect.size.height *= coeff_expansion;
//                    minRect.size.width *= coeff_expansion;
//                }
//
//                localization_rects.push_back(minRect);
//
//            }
//        }
//    }



    void Detect::findCandidates() {
        gradient_direction = Mat::zeros(resized_barcode.size(), CV_32F);
        Mat scharr_x(resized_barcode.size(), CV_32F), scharr_y(resized_barcode.size(), CV_32F), temp;
        Scharr(resized_barcode, scharr_x, CV_32F, 1, 0);
        Scharr(resized_barcode, scharr_y, CV_32F, 0, 1);
        // calculate magnitude of gradient, normalize and threshold
        magnitude(scharr_x, scharr_y, gradient_magnitude);
        threshold(gradient_magnitude, gradient_magnitude, 64, 1, CV_8U);
        imshow("mag", gradient_magnitude);
//        normalize(gradient_magnitude, gradient_magnitude, 0, 255, NormTypes::NORM_MINMAX, CV_8U);
//        adaptiveThreshold(gradient_magnitude, gradient_magnitude, 1, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 9, 0);
        integral(gradient_magnitude, integral_edges, CV_32F);

        //threshold(gradient_magnitude, gradient_magnitude, 50, 1, THRESH_BINARY);
//        threshold(gradient_magnitude, gradient_magnitude, 48, 1, THRESH_BINARY);
//        gradient_magnitude.convertTo(gradient_magnitude, CV_8U);
        for (int y = 0; y < height; y++) {
            //pixels_position.clear();
            auto *x_row = scharr_x.ptr<float_t>(y);
            auto *y_row = scharr_y.ptr<float_t>(y);
            auto *magnitude_row = gradient_magnitude.ptr<uint8_t>(y);
            int pos = 0;
            for (; pos < width; pos++) {
                if (magnitude_row[pos] == 0) {
                    x_row[pos] = 0;
                    y_row[pos] = 0;
                    continue;
                }
                if (x_row[pos] < 0) {
                    x_row[pos] *= -1;
                    y_row[pos] *= -1;
                }
            }
        }

        integral(scharr_x, temp, integral_x_sq, CV_32F, CV_32F);
        integral(scharr_y, temp, integral_y_sq, CV_32F, CV_32F);
        integral(scharr_x.mul(scharr_y), integral_xy, temp, CV_32F, CV_32F);
        float window_ratio = 0.01;
        Mat raw_consistency, orientation;
        while (window_ratio <= 0.1) {
#ifdef CV_DEBUG
            printf("window ratio: %f\n", window_ratio);
#endif

            calConsistency(raw_consistency, orientation, cvRound(min(width, height) * window_ratio));
            window_ratio += 0.01;

        }
//        adaptiveThreshold(raw_consistency,consistency,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,0);
//        threshold(raw_consistency, consistency, 220, 255, THRESH_BINARY);

//        normalize(consistency, consistency, 0, 255, NormTypes::NORM_MINMAX, CV_8U);

//        threshold(consistency, consistency, 75, 255, THRESH_BINARY);

        imshow("before-open", barcode);


//        processed_barcode = consistency;
    }

    void Detect::connectComponents() {
        // connect large components by doing morph close followed by morph open
        // use larger element size for erosion to remove small elements joined by dilation, element size is experimentally determined
        Mat small_elemSE, large_elemSE;
        int small = 8, large = 10;
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

    Mat Detect::regionGrowing(Mat &consistency, Mat &orientation, int window_size) {
        const float LOCAL_THRESHOLD_CONSISTENCY = 0.99, THRESHOLD_RADIAN = PI / 20, THRESHOLD_BLOCK_NUM =
                float(consistency.cols * consistency.rows) / 60.0, LOCAL_RATIO = 0.5;
        Point2d pToGrowing, pt;                       //待生长点位置
//        float pGrowValue;                             //待生长点灰度值
        float pSrcValue;                               //生长起点灰度值
        float pCurValue;                               //当前生长点灰度值
        float sin_sum, cos_sum, counter, edge_num;
        Mat growImage = Mat::zeros(consistency.size(), CV_8U);   //创建一个空白区域，填充为黑色
        //生长方向顺序数据
        int DIR[8][2] = {{-1, -1},
                         {0,  -1},
                         {1,  -1},
                         {1,  0},
                         {1,  1},
                         {0,  1},
                         {-1, 1},
                         {-1, 0}};
        vector<Point2f> growingPoints, growingImgPoints;
//        pSrcValue = srcImage.at<float_t>(pt.y, pt.x);         //记录生长点的灰度值
        for (int y = 0; y < consistency.rows; y++) {
            //pixels_position.clear();
            auto *consistency_row = consistency.ptr<uint8_t>(y);

            int x = 0;
            for (; x < consistency.cols; x++) {
                if (consistency_row[x] == 0)
                    continue;
                // flag
                consistency_row[x] = 0;
                growingPoints.clear();
                growingImgPoints.clear();
                growImage.setTo(0);
                growImage.at<uint8_t>(y, x) = 255;              //标记生长点
                pCurValue = orientation.at<float_t>(y, x);
                sin_sum = sin(2 * pCurValue);
                cos_sum = cos(2 * pCurValue);
                counter = 1;
                pt = Point2d(x, y);

                growingPoints.push_back(pt);
                growingImgPoints.push_back(pt);
                while (!growingPoints.empty()) {
                    pt = growingPoints.back();
                    growingPoints.pop_back();
                    pSrcValue = orientation.at<float_t>(pt.y, pt.x);

                    //growing in eight directions
                    for (int i = 0; i < 9; ++i) {
                        pToGrowing.x = pt.x + DIR[i][0];
                        pToGrowing.y = pt.y + DIR[i][1];
                        //check if out of boundary
                        if (pToGrowing.x < 0 || pToGrowing.y<0 || pToGrowing.x>(consistency.cols - 1) ||
                            (pToGrowing.y > consistency.rows - 1))
                            continue;

                        if (consistency.at<uint8_t>(pToGrowing.y, pToGrowing.x) == 0)
                            continue;
                        pCurValue = orientation.at<float_t>(pToGrowing.y, pToGrowing.x);
                        if (pCurValue < (-PI * 3 / 4)) //block gradient is not consistent
                            continue;
                        if (abs(pCurValue - pSrcValue) < THRESHOLD_RADIAN ||
                            abs(pCurValue - pSrcValue) > PI - THRESHOLD_RADIAN) {
                            growImage.at<uint8_t>(pToGrowing.y, pToGrowing.x) = 255;      //标记为白色
                            consistency.at<uint8_t>(pToGrowing.y, pToGrowing.x) = 0;
                            sin_sum += sin(2 * pCurValue);
                            cos_sum += cos(2 * pCurValue);
                            counter += 1;
                            growingPoints.push_back(pToGrowing);                 //将下一个生长点压入栈中
                            growingImgPoints.push_back(pToGrowing);
                        }
                    }
                }
                //minimum block num
                if (counter < THRESHOLD_BLOCK_NUM)
                    continue;

                float local_consistency = (sin_sum * sin_sum + cos_sum * cos_sum) / counter / counter;
                // minimum local gradient orientation consistency
                if (local_consistency < LOCAL_THRESHOLD_CONSISTENCY)
                    continue;
                Rect rect = boundingRect(growingImgPoints);
//                float ans = counter / rect.width / rect.height;
//                printf("%f\n",ans);
                if (counter < rect.width * rect.height * LOCAL_RATIO)
                    continue;
                rect.x = (rect.x) * window_size;
                rect.y = (rect.y) * window_size;
                rect.height *= window_size;
                rect.width *= window_size;
                edge_num = calcRectSum(integral_edges, min(rect.x + rect.width, width),
                                       max(rect.x, 0),
                                       max(rect.y, 0),
                                       min(rect.y + rect.height, height));
                // minimum local edge ratio
                if (edge_num > rect.height * rect.width * LOCAL_RATIO) {
                    double local_orientation = computeOrientation(cos_sum, sin_sum);
                    std::cout << local_consistency << " " << local_orientation << " " << counter << " "
                              << edge_num / rect.height / rect.width << std::endl;
                    localization_bbox.push_back(rect);
                    bbox_scores.push_back(edge_num / rect.height / rect.width);
                    bbox_orientations.push_back(local_orientation);
                    rectangle(resized_barcode, rect, 127);
                    imshow("grow image", resized_barcode);
                }


            }


        }

        return growImage.clone();
    }

    Mat Detect::calConsistency(Mat &raw_consistency, Mat &orientation, int window_size) {
        /* calculate variance of gradient directions around each pixel
        in the img_details.gradient_directions matrix
        */
        //FIXME some block's consistency is NaN
        int right_col, left_col, top_row, bottom_row;
        float xy, x_sq, y_sq, d, rect_area;
        const float THRESHOLD_AREA = float(window_size * window_size) * 0.5f, THRESHOLD_CONSISTENCY = 0.01f;
        Size new_size(width / window_size, height / window_size);
        raw_consistency = Mat(new_size, CV_8U),
                orientation = Mat(new_size, CV_32F);
        //imshow("非零积分图", gradient_magnitude);
        for (int y = 0; y < new_size.height; y++) {
            //pixels_position.clear();
            auto *consistency_row = raw_consistency.ptr<uint8_t>(y);
            auto *orientation_row = orientation.ptr<float_t>(y);
            if (y * window_size >= height) {
                continue;
            }
            top_row = y * window_size;
            bottom_row = min(height, (y + 1) * window_size);
            int pos = 0;
            for (; pos < new_size.width; pos++) {

                // then calculate the column locations of the rectangle and set them to -1
                // if they are outside the matrix bounds
                if (pos * window_size >= width)
                    continue;
                left_col = pos * window_size;
                right_col = min(width, (pos + 1) * window_size);

                //we had an integral image to count non-zero elements
                rect_area = calcRectSum(integral_edges, right_col, left_col, top_row, bottom_row);
                if (rect_area < THRESHOLD_AREA) {
                    // 有梯度的点占比小于阈值则视为平滑区域
                    consistency_row[pos] = 0;
                    orientation_row[pos] = -PI;
                    continue;
                }

                x_sq = calcRectSum(integral_x_sq, right_col, left_col, top_row, bottom_row);
                y_sq = calcRectSum(integral_y_sq, right_col, left_col, top_row, bottom_row);
                xy = calcRectSum(integral_xy, right_col, left_col, top_row, bottom_row);
                // get the values of the rectangle corners from the integral image - 0 if outside bounds
                d = sqrt((x_sq - y_sq) * (x_sq - y_sq) + 4 * xy * xy) / (x_sq + y_sq);
                if (d > THRESHOLD_CONSISTENCY) {
                    consistency_row[pos] = 255;
                    orientation_row[pos] = computeOrientation(x_sq - y_sq, 2 * xy);
                    rectangle(barcode, Point2d(left_col, top_row), Point2d(right_col, bottom_row), 255);
                } else {
                    consistency_row[pos] = 0;
                    orientation_row[pos] = -PI;
                }
//                variance.at<float_t>(y, pos) = data;
            }

        }
        imshow("consistency", raw_consistency);
        morphologyEx(raw_consistency, raw_consistency, MORPH_DILATE, getStructuringElement(MorphShapes::MORPH_RECT,
                                                                                           Size(3, 3)));
        morphologyEx(raw_consistency, raw_consistency, MORPH_ERODE, getStructuringElement(MorphShapes::MORPH_CROSS,
                                                                                          Size(3, 3)));
        processed_barcode = Mat::zeros(resized_barcode.size(), CV_32F);
        regionGrowing(raw_consistency, orientation, window_size);
        return raw_consistency;

    }


}