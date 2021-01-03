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
#include "ean13_decoder.hpp"

// three digit decode method from https://baike.baidu.com/item/EAN-13

namespace cv {
namespace barcode {

static constexpr int constexpr_bitsNum = 95;
static constexpr int constexpr_digitNumber = 13;
// default thought that mat is a matrix after binary-transfer.
/**
* decode EAN-13
* @prama: data: the input array,
* @prama: start, the index of start order, begin at 0, max-value is data.size()-1
* it scan begin at the data[start]
*/
// TODO!, need fix the param: stars's usage
Result Ean13Decoder::decode(vector<uchar> data, int start) const
{
    string result;
    char decode_result[constexpr_digitNumber + 1]{'\0'};
    if (data.size() - start < constexpr_bitsNum)
    {
        return Result("size wrong", BarcodeFormat::NONE);
    }
    try
    {
        start = findStartGuardPatterns(data).second;
        vector<int> counters = {0, 0, 0, 0};
        int end = data.size();
        uint32_t first_char_bit = 0;
        // [1,6] are left part of EAN, [7,12] are right part, index 0 is calculated by left part
        for (int i = 1; i < 7 && start < end; ++i)
        {
            int bestMatch = decodeDigit(data, counters, start, get_AB_Patterns());
            if (bestMatch == -1)
            {
                return Result("ERROR", BarcodeFormat::NONE);
            }
            decode_result[i] = static_cast<char>('0' + bestMatch % 10);
            start = std::accumulate(counters.cbegin(), counters.cend(), start);
            first_char_bit |= (bestMatch >= 10) << i;
        }
        decode_result[0] = static_cast<char>(FIRST_CHAR_ARRAY()[first_char_bit >> 2] + '0');
        // why there need >> 2?
        // first, the i in for-cycle is begin in 1
        // second, the first i = 1 is always
        start = findGuardPatterns(data, start, true, MIDDLE_PATTERN(), vector<int>(MIDDLE_PATTERN().size())).second;
        for (int i = 0; i < 6 && start < end; ++i)
        {
            int bestMatch = decodeDigit(data, counters, start, get_A_or_C_Patterns());
            if (bestMatch == -1)
            {
                return Result("ERROR", BarcodeFormat::NONE);
            }
            decode_result[i + 7] = static_cast<char>('0' + bestMatch);
            start = std::accumulate(counters.cbegin(), counters.cend(), start);
        }
        findGuardPatterns(data, start, false, BEGIN_PATTERN(), vector<int>(BEGIN_PATTERN().size()));
        result = string(decode_result);
        if (!isValid(result))
        {
            return Result("Wrong: " + result.append(string(constexpr_digitNumber - result.size(), ' ')),
                          BarcodeFormat::NONE);
        }
    } catch (GuardPatternsNotFindException &e)
    {
        return Result("ERROR", BarcodeFormat::NONE);
    }
    return Result(result, BarcodeFormat::EAN_13);
}


bool Ean13Decoder::isValid(string result) const
{
    if (result.size() != constexpr_digitNumber)
    {
        return false;
    }
    int sum = 0;
    for (int index = (int)result.size() - 2, i = 1; index >= 0; index--, i++)
    {
        int temp = result[index] - '0';
        sum += (temp + ((i & 1) != 0 ? temp << 1 : 0));
    }
    return (result.back() - '0') == ((10 - (sum % 10)) % 10);
}


Ean13Decoder::Ean13Decoder()
{
    this->bitsNum = constexpr_bitsNum;
    this->digitNumber = constexpr_digitNumber;
}
}
}