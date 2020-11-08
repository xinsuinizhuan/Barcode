//
// Created by nanos on 2020/9/30.
//

#include <opencv2/imgproc.hpp>
#include <deque>
#include "decoder/absdecoder.hpp"

namespace cv {

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

    void adaptBinaryzation(InputArray src, OutputArray& dst) {
        const int MAXIMUM = 1, MINIMUM = -1, NONE = 0;
        Mat gray_img = src.getMat();
        Mat blur;
        GaussianBlur(gray_img, blur, Size2i(3, 3), 1, 1);
        //归一化, 长度1000 pixels
        Mat norm_img, d_norm_img;
        int length = 1000;
        if(gray_img.cols > length) {
            resize(gray_img,norm_img,Size(length,gray_img.rows),0,0,INTER_AREA);
        } else {
            resize(gray_img,norm_img, Size(length,gray_img.rows),0,0,INTER_LINEAR);
        }
        norm_img.convertTo(norm_img,CV_64F);

        Matx16d d_kernel = {-1.0/3,-1.0/3,-1.0/3,1.0/3,1.0/3,1.0/3};
        Sobel(norm_img,d_norm_img,CV_64F,1,0,3);
        const uchar WHITE = 255, BLACK = 0;
        dst.create(src.size(), CV_8UC1);
        Mat result(Size(norm_img.cols, norm_img.rows),CV_8UC1);

        for(int row = 0;row < d_norm_img.rows; row ++) {

            Mat tempRow = d_norm_img.rowRange(row,row+1);
            double minVal, maxVal;
            Point minIdx, maxIdx;
            minMaxLoc(abs(tempRow),&minVal,&maxVal,&minIdx,&maxIdx);
            double threshold = 50;
            std::deque<std::pair<int,int>> extremes_que;
            for(int col = 0; col < d_norm_img.cols; col ++) {
                double temp = d_norm_img.at<double>(row,col);
                double abs_temp = abs(temp);
                int pre_extreme = NONE, abs_pre_extreme_val = 0;
                if(!extremes_que.empty()) {
                    auto back = extremes_que.back();
                    pre_extreme = back.second;
                    abs_pre_extreme_val = d_norm_img.at<double>(row,back.first);
                }
                if(abs_temp > threshold) {
                    if(temp > 0) {//可能的极大值点
                        if(pre_extreme == MAXIMUM) {// 更新极大值
                            if (abs_temp > abs_pre_extreme_val) {
                                extremes_que.pop_back();
                                extremes_que.push_back(std::pair<int, int>(col, MAXIMUM));
                            } else {// 前一个是极大值
                                //threshold = min(threshold, abs_temp/2);
                            }
                        } else {
                            extremes_que.push_back(std::pair<int, int>(col, MAXIMUM));
                        }
                    } else {// 可能的极小值点
                        if(pre_extreme == MINIMUM) {// 更新极小值
                            if (abs_temp > abs_pre_extreme_val) {
                                extremes_que.pop_back();
                                extremes_que.push_back(std::pair<int, int>(col, MINIMUM));
                            } else {
                                //threshold = min(threshold, abs_temp/2);
                            }
                        } else {
                            extremes_que.push_back(std::pair<int, int>(col, MINIMUM));
                        }
                    }
                }
            }
            //二值化
            if(extremes_que.empty()) {
                continue;
            }
            uchar signal;
            if(extremes_que.front().second == MAXIMUM) {
                signal = BLACK;
            } else {
                signal = WHITE;
            }
            for(int col = 0; col < result.cols; col ++) {
                if(!extremes_que.empty()) {
                    std::pair<int,int> extreme = extremes_que.front();
                    if(col == extreme.first) {
                        signal == BLACK?signal = WHITE:signal = BLACK;
                        extremes_que.pop_front();
                    }
                }
                result.at<uchar>(row,col) = signal;
            }
        }

        Mat mat_dst = dst.getMat();
        resize(result, mat_dst,Size(mat_dst.cols, mat_dst.rows));
    }

}