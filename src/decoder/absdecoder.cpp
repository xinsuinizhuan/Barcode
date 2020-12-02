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

#include <opencv2/imgproc.hpp>
#include <deque>
#include <opencv2/opencv.hpp>
#include "decoder/absdecoder.hpp"


namespace cv {
    //TODO 读取
    void cutImage(InputArray _src, OutputArray &_dst, RotatedRect rect) {

        Point2f vertices[4];
        rect.points(vertices);
        int height = rect.size.height;
        int width = rect.size.width;

        if(height > width) {
            height = rect.size.width;
            width = rect.size.height;
            Point2f v0 = vertices[0];
            for(int i = 0;i <= 2;i ++) {
                vertices[i] = vertices[i+1];
            }
            vertices[3] = v0;
        }
        Point2f dst_vertices[] = {
                Point2f(0, height - 1),
                Point2f(0, 0),
                Point2f(width - 1, 0),
                Point2f(width - 1, height - 1) };
        _dst.create(Size(width,height),CV_8UC1);
        Mat M = getPerspectiveTransform(vertices, dst_vertices);
        Mat dst = _dst.getMat();
        warpPerspective(_src.getMat(), dst, M, _dst.size(), cv::INTER_LINEAR, BORDER_CONSTANT, Scalar(255));
    }

void fillCounter(const std::vector<uchar> &row, int start, std::vector<int> &counters)
{
    // 先不考虑异常处理
    int counter_length = counters.size();
    std::fill(counters.begin(), counters.end(), 0);
    int end = row.size();
    if (start >= end)
    {
        // TODO throw NotFoundException.getNotFoundInstance();
    }
    uchar isWhite = row[start];
    int counterPosition = 0;
    while (start < end)
    {
        if (row[start] == isWhite)
        { // that is, exactly one is true
            counters[counterPosition]++;
        }
        else
        {
            counterPosition++;
            if (counterPosition == counter_length)
            {
                break;
            }
            else
            {
                counters[counterPosition] = 1;
                isWhite = 255 - isWhite;
            }
        }
        ++start;
    }
    if (!(counterPosition == counter_length || (counterPosition == counter_length - 1 && start == end)))
    {
        // throw a error or any others
    }
}

}