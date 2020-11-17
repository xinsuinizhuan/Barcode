//
// Created by nanos on 2020/9/30.
//

#include <opencv2/imgproc.hpp>
#include <deque>
#include <opencv2/opencv.hpp>
#include "decoder/absdecoder.hpp"


namespace cv {
    void cutImage(InputArray _src, OutputArray &_dst, RotatedRect rect) {
        Mat src = _src.getMat();
        Point2i center(src.cols / 2, src.rows / 2);
        cv::Mat t_mat = cv::Mat::zeros(2, 3, CV_64F);
        t_mat.at<double>(0, 0) = 1;
        t_mat.at<double>(1, 1) = 1;
        t_mat.at<double>(0, 2) = src.cols / 2 - rect.center.x;
        t_mat.at<double>(1, 2) = src.rows / 2 - rect.center.y;
        Mat src_copy;
        warpAffine(src, src_copy, t_mat, _src.size(), INTER_NEAREST, BORDER_CONSTANT, Scalar(255));
        double angle_offset = 0;
        Size cut_size = rect.size;
        if (rect.size.height > rect.size.width) {
            angle_offset = 90;
            cut_size.width = rect.size.height;
            cut_size.height = rect.size.width;
        }
        Mat M = getRotationMatrix2D(center, rect.angle + angle_offset, 1);
        warpAffine(src_copy, src_copy, M, _src.size(), INTER_NEAREST, BORDER_CONSTANT, Scalar(255));
        getRectSubPix(src_copy, cut_size, center, _dst);
    }

    void fillCounter(const std::vector<uchar> &row, int start, std::vector<int> &counters) {
        // 先不考虑异常处理
        int counter_length = counters.size();
        std::fill(counters.begin(), counters.end(), 0);
        int end = row.size();
        if (start >= end) {
            // TODO throw NotFoundException.getNotFoundInstance();
        }
        uchar isWhite = row[start];
        int counterPosition = 0;
        while (start < end) {
            if (row[start] == isWhite) { // that is, exactly one is true
                counters[counterPosition]++;
            } else {
                counterPosition++;
                if (counterPosition == counter_length) {
                    break;
                } else {
                    counters[counterPosition] = 1;
                    isWhite = 255 - isWhite;
                }
            }
            ++start;
        }
        if (!(counterPosition == counter_length || (counterPosition == counter_length - 1 && start == end))) {
            // throw a error or any others
        }
    }



}