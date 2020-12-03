//
// Created by nanos on 2020/9/28.
//

#ifndef BARCODE_INCLUDE_DECODER_BARCODE_COLOR_H
#define BARCODE_INCLUDE_DECODER_BARCODE_COLOR_H

#include <limits>

namespace cv {
// BLACK elemental area is 0
constexpr static int BLACK = std::numeric_limits<uchar>::min();
// WHITE elemental area is 0xff
constexpr static int WHITE = std::numeric_limits<uchar>::max();

constexpr static uint INTEGER_MATH_SHIFT = 8;
constexpr static int PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << INTEGER_MATH_SHIFT;
constexpr static int MAX_AVG_VARIANCE = static_cast<int>(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.48f);
constexpr static int MAX_INDIVIDUAL_VARIANCE = static_cast<int>(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.7f);
constexpr static uint PATTERN_LENGTH = 4;
} // namespace cv
#endif //BARCODE_INCLUDE_DECODER_BARCODE_COLOR_H
