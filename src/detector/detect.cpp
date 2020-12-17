//
// Created by 97659 on 2020/10/14.
//
#include "detector/detect.hpp"


namespace cv {
static constexpr double PI = CV_PI;
template<int x, typename Type>
struct XDividePI
{
    static constexpr Type Value = x / PI;
};
template<typename Type, int x>
struct PIDivideX
{
    static constexpr Type Value = PI / x;
};
//void Detect::normalizeRegion(RotatedRect &rect)
//{
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

//}

inline bool Detect::isValidCoord(const Point &coord, const Size &limit)
{
    if (((unsigned) coord.x < 0) || ((unsigned) coord.y < 0))
    {
        return false;
    }

    if ((unsigned) coord.x > (unsigned) (limit.width - 1) || ((unsigned) coord.y > (unsigned) (limit.height - 1)))
    {
        return false;
    }

    return true;
}

inline double Detect::computeOrientation(float y, float x)
{
    if (x >= 0)
    {
        return atan(y / x);
    }
    if (y >= 0)
    {
        return (atan(y / x) + PI);
    }
    return (atan(y / x) - PI);
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
    top_right = integral.at<float>(top_row, right_col);
    top_left = integral.at<float>(top_row, left_col);
    bottom_left = integral.at<float>(bottom_row, left_col);

    sum = (bottom_right - bottom_left - top_right + top_left);
    return sum;
}

void Detect::init(const Mat &src)
{
    barcode = src.clone();
    const double min_side = std::min(src.size().width, src.size().height);
    if (min_side > 1024.0)
    {
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
    else
    {
        purpose = UNCHANGED;
        coeff_expansion = 1.0;
        width = src.size().width;
        height = src.size().height;
        resized_barcode = barcode.clone();
    }
    medianBlur(resized_barcode, resized_barcode, 3);
#ifdef CV_DEBUG
    imshow("blurred src", resized_barcode);
#endif
}


void Detect::localization()
{
    localization_bbox.clear();
    bbox_scores.clear();

    // get integral image
    preprocess();
    #ifdef CV_DEBUG
    debug_proposals = resized_barcode.clone();
    #endif
    float window_ratio = 0.01f;
    static constexpr float window_ratio_step = 0.02f;
    static constexpr int window_ratio_stepTimes = (13 - 1) / 2; // 6 = (0.13-0.01)/0.02
    int window_size;
    for (size_t i = 0; i < window_ratio_stepTimes; i++)
    {
        window_size = cvRound(min(width, height) * window_ratio);
#ifdef CV_DEBUG
        //        printf("window ratio: %f\n", window_ratio);
        //        debug_img = resized_barcode.clone();
#endif
        calConsistency(window_size);
        #ifdef CV_DEBUG
        //        imshow("block img " + std::to_string(window_ratio), debug_img);
        #endif
        barcodeErode();
#ifdef CV_DEBUG
        //        debug_img = resized_barcode.clone();
        //        for (int y = 0; y < consistency.rows; y++)
        //        {
        //            //pixels_position.clear();
        //            auto *consistency_row = consistency.ptr<uint8_t>(y);
        //
        //            int x = 0;
        //            for (; x < consistency.cols; x++)
        //            {
        //                if (consistency_row[x] == 0)
        //                { continue; }
        //                rectangle(debug_img, Point2d(x * window_size, y * window_size),
        //                          Point2d(min((x + 1) * window_size, width), min((y + 1) * window_size, height)), 255);
        //
        //            }
        //        }
        //        imshow("erode block " + std::to_string(window_ratio), debug_img);
#endif
        regionGrowing(window_size);
        window_ratio += window_ratio_step;
    }
    #ifdef CV_DEBUG
    //    imshow("grow image", debug_proposals);
    #endif

}

bool Detect::computeTransformationPoints()
{
    bbox_indices.clear();
    transformation_points.clear();
    transformation_points.reserve(bbox_indices.size());
    RotatedRect rect;
    Point2f temp[4];
    const float THRESHOLD_SCORE = float(width * height) / 1000;
    dnn::NMSBoxes(localization_bbox, bbox_scores, THRESHOLD_SCORE, 0.1, bbox_indices);

    for (const auto &bbox_index : bbox_indices)
    {
        rect = localization_bbox[bbox_index];
        if (purpose == ZOOMING)
        {
            rect.center.x /= coeff_expansion;
            rect.center.y /= coeff_expansion;
            rect.size.height /= coeff_expansion;
            rect.size.width /= coeff_expansion;
        }
        else if (purpose == SHRINKING)
        {
            rect.center.x *= coeff_expansion;
            rect.center.y *= coeff_expansion;
            rect.size.height *= coeff_expansion;
            rect.size.width *= coeff_expansion;
        }
        rect.points(temp);
        //vector<Point2f> points;
        transformation_points.emplace_back(vector<Point2f>{temp[0], temp[1], temp[2], temp[3]});
    }

    return !transformation_points.empty();
}


void Detect::preprocess()
{
    Mat scharr_x(resized_barcode.size(), CV_32F), scharr_y(resized_barcode.size(), CV_32F), temp;
    Scharr(resized_barcode, scharr_x, CV_32F, 1, 0);
    Scharr(resized_barcode, scharr_y, CV_32F, 0, 1);
    // calculate magnitude of gradient, normalize and threshold
    magnitude(scharr_x, scharr_y, gradient_magnitude);
    threshold(gradient_magnitude, gradient_magnitude, 48, 1, THRESH_BINARY);
    #ifdef CV_DEBUG
    imshow("mag", gradient_magnitude);
    #endif
    gradient_magnitude.convertTo(gradient_magnitude, CV_8U);

//        normalize(gradient_magnitude, gradient_magnitude, 0, 255, NormTypes::NORM_MINMAX, CV_8U);
//        adaptiveThreshold(gradient_magnitude, gradient_magnitude, 1, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 9, 0);
    integral(gradient_magnitude, integral_edges, CV_32F);

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


}

void Detect::calConsistency(int window_size)
{
    static constexpr float THRESHOLD_CONSISTENCY = 0.85f;
    int right_col, left_col, top_row, bottom_row;
    float xy, x_sq, y_sq, d, rect_area;
    const float THRESHOLD_AREA = float(window_size * window_size) * 0.5f;
    Size new_size(width / window_size, height / window_size);
    consistency = Mat(new_size, CV_8U), orientation = Mat(new_size, CV_32F), edge_nums = Mat(new_size, CV_32F);
    //imshow("非零积分图", gradient_magnitude);
    for (int y = 0; y < new_size.height; y++)
    {
        //pixels_position.clear();
        auto *consistency_row = consistency.ptr<uint8_t>(y);
        auto *orientation_row = orientation.ptr<float_t>(y);
        auto *edge_nums_row = edge_nums.ptr<float_t>(y);
        if (y * window_size >= height)
        {
            continue;
        }
        top_row = y * window_size;
        bottom_row = min(height, (y + 1) * window_size);
        int pos = 0;
        for (; pos < new_size.width; pos++)
        {

            // then calculate the column locations of the rectangle and set them to -1
            // if they are outside the matrix bounds
            if (pos * window_size >= width)
            {
                continue;
            }
            left_col = pos * window_size;
            right_col = min(width, (pos + 1) * window_size);

            //we had an integral image to count non-zero elements
            rect_area = calcRectSum(integral_edges, right_col, left_col, top_row, bottom_row);
            if (rect_area < THRESHOLD_AREA)
            {
                // 有梯度的点占比小于阈值则视为平滑区域
                consistency_row[pos] = 0;
                continue;
            }

            x_sq = calcRectSum(integral_x_sq, right_col, left_col, top_row, bottom_row);
            y_sq = calcRectSum(integral_y_sq, right_col, left_col, top_row, bottom_row);
            xy = calcRectSum(integral_xy, right_col, left_col, top_row, bottom_row);

            // get the values of the rectangle corners from the integral image - 0 if outside bounds
            d = sqrt((x_sq - y_sq) * (x_sq - y_sq) + 4 * xy * xy) / (x_sq + y_sq);
            if (d > THRESHOLD_CONSISTENCY)
            {
                consistency_row[pos] = 255;
                orientation_row[pos] = computeOrientation(x_sq - y_sq, 2 * xy) / 2.0;
                edge_nums_row[pos] = rect_area;
                #ifdef CV_DEBUG
                //                rectangle(debug_img, Point2d(left_col, top_row), Point2d(right_col, bottom_row), 255);
                #endif
            }
            else
            {
                consistency_row[pos] = 0;
            }
        }
    }
}

void Detect::regionGrowing(int window_size)
{
    static constexpr float LOCAL_THRESHOLD_CONSISTENCY = 0.98, THRESHOLD_RADIAN = PIDivideX<float, 40>::Value, THRESHOLD_BLOCK_NUM = 20, LOCAL_RATIO = 0.4;
    Point pt_to_grow, pt;                       //待生长点位置

    float src_value;                               //生长起点灰度值
    float cur_value;                               //当前生长点灰度值
    float edge_num;
    double rect_orientation;
    float sin_sum, cos_sum, counter;
    //生长方向顺序数据
    static constexpr int DIR[8][2] = {{-1, -1},
                                      {0,  -1},
                                      {1,  -1},
                                      {1,  0},
                                      {1,  1},
                                      {0,  1},
                                      {-1, 1},
                                      {-1, 0}};
    vector<Point2f> growingPoints, growingImgPoints;
    for (int y = 0; y < consistency.rows; y++)
    {
        //pixels_position.clear();
        auto *consistency_row = consistency.ptr<uint8_t>(y);

        int x = 0;
        for (; x < consistency.cols; x++)
        {
            if (consistency_row[x] == 0)
            {
                continue;
            }
            // flag
            consistency_row[x] = 0;
            growingPoints.clear();
            growingImgPoints.clear();
            pt = Point2d(x, y);
            cur_value = orientation.at<float_t>(pt);
            sin_sum = sin(2 * cur_value);
            cos_sum = cos(2 * cur_value);
            counter = 1;
            edge_num = edge_nums.at<float_t>(pt);
            growingPoints.push_back(pt);
            growingImgPoints.push_back(pt);
            while (!growingPoints.empty())
            {
                pt = growingPoints.back();
                growingPoints.pop_back();
                src_value = orientation.at<float_t>(pt);

                //growing in eight directions
                for (int i = 0; i < 9; ++i)
                {
                    pt_to_grow.x = pt.x + DIR[i][0];
                    pt_to_grow.y = pt.y + DIR[i][1];
                    //check if out of boundary
                    if (!isValidCoord(pt_to_grow, consistency.size()))
                    {
                        continue;
                    }

                    if (consistency.at<uint8_t>(pt_to_grow) == 0)
                    {
                        continue;
                    }
                    cur_value = orientation.at<float_t>(pt_to_grow);
                    if (abs(cur_value - src_value) < THRESHOLD_RADIAN ||
                        abs(cur_value - src_value) > PI - THRESHOLD_RADIAN)
                    {
                        consistency.at<uint8_t>(pt_to_grow) = 0;
                        sin_sum += sin(2 * cur_value);
                        cos_sum += cos(2 * cur_value);
                        counter += 1;
                        edge_num += edge_nums.at<float_t>(pt_to_grow);
                        growingPoints.push_back(pt_to_grow);                 //将下一个生长点压入栈中
                        growingImgPoints.push_back(pt_to_grow);
                    }
                }
            }
            //minimum block num
            if (counter < THRESHOLD_BLOCK_NUM)
            {
                continue;
            }
            float local_consistency = (sin_sum * sin_sum + cos_sum * cos_sum) / counter / counter;
            // minimum local gradient orientation consistency
            if (local_consistency < LOCAL_THRESHOLD_CONSISTENCY)
            {
                continue;
            }
            RotatedRect minRect = minAreaRect(growingImgPoints);
            if (edge_num < minRect.size.area() * float(window_size * window_size) * LOCAL_RATIO ||
                counter < minRect.size.area() * LOCAL_RATIO)
            {
                continue;
            }
//                printf("counter ratio: %f\n", counter / rect.size.area());
            const double local_orientation = computeOrientation(cos_sum, sin_sum) / 2.0;
            // only orientation is approximately equal to the rectangle orientation
            rect_orientation = (minRect.angle) * PIDivideX<double, 180>::Value;
            if (minRect.size.width < minRect.size.height)
            {
                rect_orientation += (rect_orientation <= 0 ? PIDivideX<double, 2>::Value
                                                           : PIDivideX<double, -2>::Value);
                std::swap(minRect.size.width, minRect.size.height);
            }
            if (abs(local_orientation - rect_orientation) > THRESHOLD_RADIAN &&
                abs(local_orientation - rect_orientation) < PI - THRESHOLD_RADIAN)
            {
                continue;
            }
            minRect.angle = static_cast<float>(local_orientation) * XDividePI<180, float>::Value;
            minRect.size.width *= float(window_size + 1);
            minRect.size.height *= float(window_size + 1);
            minRect.center.x = (minRect.center.x + 0.5) * window_size;
            minRect.center.y = (minRect.center.y + 0.5) * window_size;
#ifdef CV_DEBUG
            std::cout << local_consistency << " " << local_orientation << " " << edge_num / (float) (width * height)
                      << " " << edge_num / minRect.size.area() << std::endl;
#endif
            localization_bbox.push_back(minRect);


            bbox_scores.push_back(edge_num);
            #ifdef CV_DEBUG
            //            Point2f vertices[4];
            //            minRect.points(vertices);
            //            for (int i = 0; i < 4; i++)
            //                line(debug_proposals, vertices[i], vertices[(i + 1) % 4], 127, 2);
            #endif
        }
    }
//        return growImage.clone();
}

inline const std::array<Mat, 4> &getStructuringElement()
{
    static const std::array<Mat, 4> structuringElement{
            Mat_<uint8_t>{{3,   3},
                          {255, 0, 0, 0, 0, 0, 0, 0, 255}}, Mat_<uint8_t>{{3, 3},
                                                                          {0, 0, 255, 0, 0, 0, 255, 0, 0}},
            Mat_<uint8_t>{{3, 3},
                          {0, 0, 0, 255, 0, 255, 0, 0, 0}}, Mat_<uint8_t>{{3, 3},
                                                                          {0, 255, 0, 0, 0, 0, 0, 255, 0}}};
    return structuringElement;
}

void Detect::barcodeErode()
{
    static const std::array<Mat, 4> &structuringElement = getStructuringElement();
    Mat m0, m1, m2, m3;
    dilate(consistency, m0, structuringElement[0]);
    dilate(consistency, m1, structuringElement[1]);
    dilate(consistency, m2, structuringElement[2]);
    dilate(consistency, m3, structuringElement[3]);
    int sum;
    for (int y = 0; y < consistency.rows; y++)
    {
        auto consistency_row = consistency.ptr<uint8_t>(y);
        auto m0_row = m0.ptr<uint8_t>(y);
        auto m1_row = m1.ptr<uint8_t>(y);
        auto m2_row = m2.ptr<uint8_t>(y);
        auto m3_row = m3.ptr<uint8_t>(y);

        int pos;
        for (pos = 0; pos < consistency.cols; pos++)
        {
            if (consistency_row[pos] == 0)
            {
                continue;
            }
            sum = m0_row[pos] + m1_row[pos] + m2_row[pos] + m3_row[pos];
            //more than 2 group
            consistency_row[pos] = sum > 600 ? 255 : 0;
        }
    }
}


}