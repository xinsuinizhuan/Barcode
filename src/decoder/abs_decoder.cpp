// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
// Copyright (c) 2020-2021 darkliang wangberlinT Certseeds

#include "../precomp.hpp"
#include "abs_decoder.hpp"

namespace cv {
namespace barcode {

void cropROI(const Mat &src, Mat &dst, const std::vector<Point2f> &rects)
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
            Point2f(0, (float) (height - 1)), Point2f(0, 0), Point2f((float) (width - 1), 0),
            Point2f((float) (width - 1), (float) (height - 1))};
    dst.create(Size(width, height), CV_8UC1);
    Mat M = getPerspectiveTransform(vertices, dst_vertices);
    warpPerspective(src, dst, M, dst.size(), cv::INTER_LINEAR, BORDER_CONSTANT, Scalar(255));
}

void fillCounter(const std::vector<uchar> &row, uint start, std::vector<int> &counters)
{
    size_t counter_length = counters.size();
    std::fill(counters.begin(), counters.end(), 0);
    size_t end = row.size();
    if (start >= end)
    {
    }
    uchar isWhite = row[start];
    uint counterPosition = 0;
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
    }
}

static inline int
patternMatchVariance(std::vector<int> counters, const std::vector<int> &pattern, int maxIndividualVariance)
{
    size_t numCounters = counters.size();
    int total = std::accumulate(counters.cbegin(), counters.cend(), 0);
    int patternLength = std::accumulate(pattern.cbegin(), pattern.cend(), 0);
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

    int unitBarWidth = (total << INTEGER_MATH_SHIFT) / patternLength;
    maxIndividualVariance = (maxIndividualVariance * unitBarWidth) >> INTEGER_MATH_SHIFT;
    int totalVariance = 0;
    for (uint x = 0; x < numCounters; x++)
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
    return patternMatchVariance(std::move(counters), pattern, maxIndividual);
}
}
}
