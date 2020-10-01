//
// Created by nanos on 2020/10/1.
//

#include "decoder/patternmatch.hpp"

namespace cv {
    // TODO
    // ADD more functions
    int patternMatch(std::vector<int> counters,
                     const std::vector<int> &pattern,
                     int maxIndividual) {
        return patternMatchVariance(std::move(counters), pattern, maxIndividual);
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
    inline int patternMatchVariance(std::vector<int> counters,
                                    const std::vector<int> &pattern,
                                    int maxIndividualVariance) {
        int numCounters = counters.size();
        uint total = std::accumulate(counters.cbegin(), counters.cend(), 0);
        uint patternLength = std::accumulate(pattern.cbegin(), pattern.cend(), 0);
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
            int variance = std::abs(counter - scaledPattern);
            if (variance > maxIndividualVariance) {
                return std::numeric_limits<int32_t>::max();
            }
            totalVariance += variance;
        }
        return totalVariance / total;
    }
}