//
// Created by nanos on 2020/9/30.
//
#include "decoder/AbsDecoder.hpp"

namespace cv {

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
    constexpr static uint INTEGER_MATH_SHIFT = 8;

    int patternMatchVariance(std::vector<int> counters,
                             std::vector<int> pattern,
                             int maxIndividualVariance) {
        int numCounters = counters.size();
        uint total = 0;
        uint patternLength = 0;
        for (const auto &item : counters) {
            total += item;
        }
        for (const auto &item : pattern) {
            patternLength += item;
        }
        if (total < patternLength) {
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
        for (int x = 0; x < numCounters; x++) {
            int counter = counters[x] << INTEGER_MATH_SHIFT;
            int scaledPattern = pattern[x] * unitBarWidth;
            int variance = counter > scaledPattern ? counter - scaledPattern : scaledPattern - counter;
            if (variance > maxIndividualVariance) {
                return std::numeric_limits<int32_t>::max();
            }
            totalVariance += variance;
        }
        return totalVariance / total;
    }
}