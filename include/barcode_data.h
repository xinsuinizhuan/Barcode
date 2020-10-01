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
}

#endif //BARCODE_INCLUDE_DECODER_BARCODE_COLOR_H
