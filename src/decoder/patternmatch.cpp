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

#include <iostream>
#include "decoder/patternmatch.hpp"

namespace cv {
// TODO: ADD more functions
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

inline int
patternMatchConsieDistance(std::vector<int> counters, const std::vector<int> &pattern, uint maxIndividualVariance)
{
    uint total = std::accumulate(counters.cbegin(), counters.cend(), 0);
    uint pattern_length = std::accumulate(pattern.cbegin(), pattern.cend(), 0);
    if (total < pattern_length)
    {
        return std::numeric_limits<int32_t>::max();
    }
    int inner_result =
            static_cast<uint>(std::inner_product(std::cbegin(counters), std::cend(counters), std::cbegin(pattern), 0))
                    << INTEGER_MATH_SHIFT;
    int divide_first = std::inner_product(std::cbegin(counters), std::cend(counters), std::cbegin(counters), 0);
    int divide_second = std::inner_product(std::cbegin(pattern), std::cend(pattern), std::cbegin(pattern), 0);
    return (1 << INTEGER_MATH_SHIFT) - static_cast<int>(inner_result / std::sqrt(divide_first * divide_second));
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
}