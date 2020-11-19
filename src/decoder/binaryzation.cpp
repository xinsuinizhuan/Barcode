#include "decoder/binaryzation.hpp"

namespace cv {
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
                        signal = ((signal == BLACK) ? WHITE : BLACK);
                        extremes_que.pop_front();
                    }
                }
                result.at<uchar>(row, col) = signal;
            }
        }

        Mat mat_dst = dst.getMat();
        resize(result, mat_dst, Size(mat_dst.cols, mat_dst.rows), 0, 0, INTER_NEAREST);
    }

    // w has to be odd
    void getMinMax(int row, int col, int w , Mat img, uchar& min, uchar&max) {
        int half_width = w/2;
        min = 255;
        max = 0;
        for (int offset_row = -1 * half_width; offset_row <= half_width; offset_row ++) {
            int temp_row = row + offset_row;
            if (temp_row < 0 || temp_row >= img.rows) {
                continue;
            }
            for (int offset_col = -1 * half_width; offset_col <= half_width; offset_col ++) {
                int temp_col = col + offset_col;
                if (temp_col < 0 || temp_col >= img.cols) {
                    continue;
                }
                uchar temp_val = img.at<uchar>(temp_row, temp_col);
                if(temp_val > max) {
                    max = temp_val;
                }
                if(temp_val < min) {
                    min = temp_val;
                }
            }
        }
    }

    void enhanceLocalBinaryzation(InputArray src, OutputArray &dst, int window_size, float alpha) {
        Mat _src = src.getMat();
        dst.createSameSize(src, CV_8UC1);
        Mat _dst = dst.getMat();
        Mat hist;
        uint total_pixel = _src.cols * _src.rows;
        float range[] = {0, 256};
        const float *histRange = {range};
        int hist_size = 256;
        calcHist(&_src, 1, 0, Mat(), hist, 1, &hist_size, &histRange);
//        int hist_w = 512, hist_h = 400;
//        int bin_w = cvRound((double) hist_w / hist_size);
//        Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
//
//        normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
//
//        for (int i = 1; i < hist_size; i++) {
//            line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
//                 Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
//                 Scalar(255, 0, 0), 2, 8, 0);
//        }
//
//        imshow("Source image", _src);
//        imshow("calcHist Demo", histImage);

        cv::Mat accumulatedHist = hist.clone();
        //Eliminate grayscale outliers
        int threshold1 = -1;
        const uint A = total_pixel * 0.1;
        for (int i = hist_size - 2; i >= 0; i--) {
            accumulatedHist.at<float>(i) += accumulatedHist.at<float>(i + 1);
            if (accumulatedHist.at<float>(i) >= A) {
                threshold1 = i;
                break;
            }
        }

        Mat threshold2(_src.rows, _src.cols, CV_32FC1, Scalar(0));
        Mat threshold3(_src.rows, _src.cols, CV_32FC1, Scalar(0));
        for (int row = 0; row < threshold2.rows; row ++) {
            for (int col = 0; col < threshold2.cols; col ++) {
                uchar min, max;
                getMinMax(row, col, window_size, _src, min, max);
                threshold2.at<float>(row, col) = (min+max)/2.0f;
                threshold3.at<float>(row, col) = max - min;
            }
        }
        Mat threshold4;
        GaussianBlur(threshold2, threshold4, Size(window_size,window_size), window_size/2,window_size/2);

        //Binaryzation
        float t1 = (1 + alpha) * threshold1;
        float t2 = (1 - alpha) * threshold1;
        float t3 = alpha * threshold1;
        for (int row = 0; row < _src.rows; row ++) {
            for ( int col = 0; col < _src.cols; col ++) {
                uchar temp_val = _src.at<uchar>(row, col);
                if(temp_val > t1) {
                    _dst.at<uchar>(row, col) = 255;
                } else if(temp_val < t2) {
                    _dst.at<uchar>(row, col) = 0;
                } else {
                    if (threshold3.at<float>(row, col) > t3) {
                        if (temp_val < threshold4.at<float>(row, col)) {
                            _dst.at<uchar>(row, col) = 0;
                        } else {
                            _dst.at<uchar>(row, col) = 255;
                        }
                    } else {
                        float threshold3_temp = (threshold1 + threshold4.at<float>(row, col))/2.0f;
                        threshold3.at<float>(row, col) = threshold3_temp;
                        if(temp_val < threshold3_temp) {
                            _dst.at<uchar>(row, col) = 0;
                        } else {
                            _dst.at<uchar>(row, col) = 255;
                        }
                    }
                }
            }
        }



    }
}

