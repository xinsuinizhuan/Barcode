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
//
// Created by nanos on 2020/10/1.
//
#ifndef __OPENCV_BARCODE_PATTERNMATCH_HPP__
#define __OPENCV_BARCODE_PATTERNMATCH_HPP__

#include <vector>
#include <numeric>
#include <utility>
#include "opencv2/core/mat.hpp"
#include "barcode_data.hpp"

namespace cv {
int patternMatch(std::vector<int> counters, const std::vector<int> &pattern, uint maxIndividual);

inline int
patternMatchConsieDistance(std::vector<int> counters, const std::vector<int> &pattern, uint maxIndividualVariance);

inline int patternMatchVariance(std::vector<int> counters, const std::vector<int> &pattern, int maxIndividualVariance);
};

#endif //! __OPENCV_BARCODE_PATTERNMATCH_HPP__
