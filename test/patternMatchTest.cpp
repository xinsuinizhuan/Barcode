//
// Created by nanos on 2020/11/10.
//

#include "test_precomp.hpp"
#include "decoder/patternmatch.hpp"
#include "decoder/ean_decoder.hpp"

using std::vector;
TEST(patternMatch, build_objects) {
    using namespace cv;
}

TEST(patternMatch, compare) {
    using namespace cv;
    const vector<int> prepares = {2, 3, 1, 1};
    static const vector<vector<int>> A_or_C_Patterns = {
            {3, 2, 1, 1}, // 0
            {2, 2, 2, 1}, // 1
            {2, 1, 2, 2}, // 2
            {1, 4, 1, 1}, // 3
            {1, 1, 3, 2}, // 4
            {1, 2, 3, 1}, // 5
            {1, 1, 1, 4}, // 6
            {1, 3, 1, 2}, // 7
            {1, 2, 1, 3}, // 8
            {3, 1, 1, 2}  // 9
    };
    static const vector<vector<int>> AB_Patterns = [] {
        auto AB_Patterns_inited = vector<vector<int>>(20, vector<int>(PATTERN_LENGTH, 0));
        std::copy(A_or_C_Patterns.cbegin(), A_or_C_Patterns.cend(), AB_Patterns_inited.begin());
        //AB pattern is
        constexpr int offset = 10;
        for (int i = 0; i < A_or_C_Patterns.size(); ++i) {
            for (int j = 0; j < PATTERN_LENGTH; ++j) {
                AB_Patterns_inited[i + offset][j] = AB_Patterns_inited[i][PATTERN_LENGTH - j - 1];
            }
        }
        return AB_Patterns_inited;
    }();
    auto maxmium = std::numeric_limits<int32_t>::max();
    for (const auto &i: AB_Patterns) {
        for (int j = 0; j < 4; ++j) {
            std::cout << i[j] << " ";
        }
        auto temp = cv::patternMatchVariance(prepares, i, MAX_INDIVIDUAL_VARIANCE);
        std::cout << temp << " ";
        maxmium = std::min(temp, maxmium);
        std::cout << maxmium << std::endl;
    }
}

TEST(patternMatch, Pictrue) {
    cv::BarcodeDetector bardet;
    std::string graphs[] = {"ZXing2.png"};
    cv::Point2f points[4];
    //
    for (const auto &i: graphs) {
        std::cout << i << '\n';
        cv::Mat frame = cv::imread(R"(./../../test/data/)" + i);
        std::vector<cv::RotatedRect> rects;
        try {
            bardet.detect(frame, rects);
        }
        catch (cv::Exception &ex) {
            std::cerr << ex.what() << "No detect pictures\n";
            continue;
        }
        for (const auto &rect : rects) {
            rect.points(points);
            for (int j = 0; j < 4; j++) {
#ifdef CV_DEBUG
                cv::line(frame, points[j % 4], points[(j + 1) % 4], cv::Scalar(0, 255, 0));
#endif
            }
        }
        std::vector<std::string> results;
        try {
            bardet.decode(frame, rects, results);
        }
        catch (cv::Exception &ex) {
            std::cerr << ex.what() << "No detect results\n";
            continue;
        }
        for (const auto &result : results) {
            std::cout << result << std::endl;
        }
#ifdef CV_DEBUG
        cv::imshow(i, frame);
        cv::waitKey(0);
#endif
    }
}