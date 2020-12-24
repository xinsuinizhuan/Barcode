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
#include "opencv2/decoder/abs_decoder.hpp"

namespace cv {

void cutImage(InputArray _src, OutputArray &_dst, const std::vector<Point2f> &rects)
{
    std::vector<Point2f> vertices = rects;
    int height = cvRound(norm(vertices[0] - vertices[1]));
    int width = cvRound(norm(vertices[1] - vertices[2]));
    if (height > width)
    {
        std::swap(height, width);
        Point2f v0 = vertices[0];
        vertices.erase(vertices.begin());
        vertices.push_back(v0);
    }
    std::vector<Point2f> dst_vertices{
            Point2f(0, height - 1), Point2f(0, 0), Point2f(width - 1, 0), Point2f(width - 1, height - 1)};
    _dst.create(Size(width, height), CV_8UC1);
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

/**
 * Determines how closely a set of observed counts of runs of black/white values matches a given
 * target pattern. This is reported as the ratio of the total variance from the expected pattern
 * proportions across all pattern elements, to the length of the pattern.
 *
 * @param counters observed counters
 * @param pattern expected pattern
 * @param maxIndividualVariance The most any counter can differ before we give up
 * @return ratio of total variance between counters and pattern compared to total pattern size,
 *  where the ratio has been multiplied by 256. So, 0 means no variance (perfect match); 256 means
 *  the total variance between counters and patterns equals the pattern length, higher values mean
 *  even more variance
 */
int patternMatch(std::vector<int> counters, const std::vector<int> &pattern, uint maxIndividual)
{
    CV_Assert(counters.size() == pattern.size());
    //return patternMatchConsieDistance(std::move(counters), pattern, maxIndividual);
    return patternMatchVariance(std::move(counters), pattern, maxIndividual);
}


static inline int patternMatchVariance(std::vector<int> counters, const std::vector<int> &pattern, int maxIndividualVariance)
{
    int numCounters = counters.size();
    uint total = std::accumulate(counters.cbegin(), counters.cend(), 0);
    uint patternLength = std::accumulate(pattern.cbegin(), pattern.cend(), 0);
    if (total < patternLength)
    {
        // If we don't even have one pixel per unit of bar width, assume this is too small
        // to reliably match, so fail:
        // and use constexpr functions
        return std::numeric_limits<int32_t>::max();
    }
    // We're going to fake floating-point math in integers. We just need to use more bits.
    // Scale up patternLength so that intermediate values below like scaledCounter will have
    // more "significant digits"

    uint unitBarWidth = (total << INTEGER_MATH_SHIFT) / patternLength;
    maxIndividualVariance = (maxIndividualVariance * unitBarWidth) >> INTEGER_MATH_SHIFT;
    int totalVariance = 0;
    for (int x = 0; x < numCounters; x++)
    {
        int counter = counters[x] << INTEGER_MATH_SHIFT;
        int scaledPattern = pattern[x] * unitBarWidth;
        int variance = std::abs(counter - scaledPattern);
        if (variance > maxIndividualVariance)
        {
            return std::numeric_limits<int32_t>::max();
        }
        totalVariance += variance;
    }
    return totalVariance / total;
}


std::ostream & operator<<(std::ostream & out, BarcodeFormat format)
{
    switch (format)
    {
        case BarcodeFormat::EAN_8:
            out << "EAN_8";
            break;
        case BarcodeFormat::EAN_13:
            out << "EAN_13";
            break;
        case BarcodeFormat::UPC_E:
            out << "UPC_E";
            break;
        case BarcodeFormat::UPC_A:
            out << "UPC_A";
            break;
        case BarcodeFormat::UPC_EAN_EXTENSION:
            out << "UPC_EAN_EXTENSION";
            break;
        default:
            out << "NONE";
    }
    return out;
}
}
