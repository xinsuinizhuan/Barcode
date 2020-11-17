//
// Created by nanos on 2020/9/30.
//

#include <opencv2/imgproc.hpp>
#include <deque>
#include <opencv2/opencv.hpp>
#include "decoder/absdecoder.hpp"
//test
#include <fstream>
#include <iostream>
#include <sstream>

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

    void adaptBinaryzation(InputArray src, OutputArray &dst) {
        const int MAXIMUM = 1, MINIMUM = -1, NONE = 0;
        const uchar WHITE = 255, BLACK = 0;
        Mat gray_img = src.getMat();
        Mat blur;
        GaussianBlur(gray_img, blur, Size2i(3, 3), 1, 1);
        //normalization to 1000 pixels
        Mat norm_img, d_norm_img;
        int length = 1000;
        if (gray_img.cols > length) {
            resize(gray_img, norm_img, Size(length, gray_img.rows), 0, 0, INTER_AREA);
        } else {
            resize(gray_img, norm_img, Size(length, gray_img.rows), 0, 0, INTER_LINEAR);
        }
        norm_img.convertTo(norm_img, CV_64F);

        Matx16d d_kernel = {-1.0 / 3, -1.0 / 3, -1.0 / 3, 1.0 / 3, 1.0 / 3, 1.0 / 3};
        Sobel(norm_img, d_norm_img, CV_64F, 1, 0, 3);
        Mat center = d_norm_img.rowRange(d_norm_img.rows / 2, d_norm_img.rows / 2 + 1);
        //写入文件
#ifdef CV_DEBUG
        using namespace std;
        ofstream outFile;
        outFile.open("./../../plot/data.csv", ios::out);
        for (int i = 0; i < center.cols; i++) {
            outFile << center.at<double>(0, i) << ",";
        }
        outFile << '\n';
        outFile.close();
#endif

        dst.create(src.size(), CV_8UC1);
        Mat result(Size(norm_img.cols, norm_img.rows), CV_8UC1);

        for (int row = 0; row < d_norm_img.rows; row++) {

            Mat tempRow = d_norm_img.rowRange(row, row + 1);
            double minVal, maxVal;
            Point minIdx, maxIdx;
            minMaxLoc(abs(tempRow), &minVal, &maxVal, &minIdx, &maxIdx);
            double threshold = 50;
            std::deque<std::pair<int, int>> extremes_que;
            for (int col = 0; col < d_norm_img.cols; col++) {
                double temp = d_norm_img.at<double>(row, col);
                double abs_temp = abs(temp);
                int pre_extreme = NONE, abs_pre_extreme_val = 0;
                if (!extremes_que.empty()) {
                    auto back = extremes_que.back();
                    pre_extreme = back.second;
                    abs_pre_extreme_val = d_norm_img.at<double>(row, back.first);
                }
                if (abs_temp > threshold) {
                    if (temp > 0) {//可能的极大值点
                        if (pre_extreme == MAXIMUM) {// 更新极大值
                            if (abs_temp > abs_pre_extreme_val) {
                                extremes_que.pop_back();
                                extremes_que.emplace_back(col, MAXIMUM);
                            } else {// 前一个是极大值
                                //threshold = min(threshold, abs_temp/2);
                            }
                        } else {
                            extremes_que.emplace_back(col, MAXIMUM);
                        }
                    } else {// 可能的极小值点
                        if (pre_extreme == MINIMUM) {// 更新极小值
                            if (abs_temp > abs_pre_extreme_val) {
                                extremes_que.pop_back();
                                extremes_que.emplace_back(col, MINIMUM);
                            } else {
                                //threshold = min(threshold, abs_temp/2);
                            }
                        } else {
                            extremes_que.emplace_back(col, MINIMUM);
                        }
                    }
                }
            }
            //二值化
            uchar signal;
            if (extremes_que.empty()) {
                signal = WHITE;
            } else if (extremes_que.front().second == MAXIMUM) {
                signal = BLACK;
            } else {
                signal = WHITE;
            }
            for (int col = 0; col < result.cols; col++) {
                if (!extremes_que.empty()) {
                    std::pair<int, int> extreme = extremes_que.front();
                    if (col == extreme.first) {
                        signal = (signal == BLACK) ? WHITE : BLACK;
                        extremes_que.pop_front();
                    }
                }
                result.at<uchar>(row, col) = signal;
            }
        }
        imshow("result_no_resize", result);

        Mat mat_dst = dst.getMat();
        resize(result, mat_dst, Size(mat_dst.cols, mat_dst.rows), 0, 0, INTER_NEAREST);
    }

}