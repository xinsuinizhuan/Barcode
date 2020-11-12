//
// Created by nanos on 2020/10/1.
//

#ifndef BARCODE_INCLUDE_DECODER_PATTERNMATCH_H
#define BARCODE_INCLUDE_DECODER_PATTERNMATCH_H

#include <vector>
#include <numeric>
#include <utility>
#include "opencv2/core/mat.hpp"
#include "barcode_data.hpp"

namespace cv {
    int patternMatch(std::vector<int> counters,
                     const std::vector<int> &pattern,
                     uint maxIndividual);

    inline int patternMatchConsieDistance(std::vector<int> counters,
                                          const std::vector<int> &pattern,
                                          uint maxIndividualVariance);

    inline int patternMatchVariance(std::vector<int> counters,
                                    const std::vector<int> &pattern,
                                    int maxIndividualVariance);
};

#endif //! BARCODE_INCLUDE_DECODER_PATTERNMATCH_H
