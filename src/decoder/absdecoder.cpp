//
// Created by nanos on 2020/9/30.
//

#include "decoder/absdecoder.hpp"

namespace cv {

    void fillCounter(const std::vector<uchar> &row, int start, std::vector<int> &counters) {
        // 先不考虑异常处理
        int counter_length = counters.size();
        std::fill(counters.begin(), counters.end(), 0);
        int end = row.size();
        if (start >= end) {
            // TODO throw NotFoundException.getNotFoundInstance();
        }
        uchar isWhite = row[start];
        int counterPosition = 0;
        while (start < end) {
            if (row[start] == isWhite) { // that is, exactly one is true
                counters[counterPosition]++;
            } else {
                counterPosition++;
                if (counterPosition == counter_length) {
                    break;
                } else {
                    counters[counterPosition] = 1;
                    isWhite = 255 - isWhite;
                }
            }
            ++start;
        }
        if (!(counterPosition == counter_length || (counterPosition == counter_length - 1 && start == end))) {
            // throw a error or any others
        }
    }

}