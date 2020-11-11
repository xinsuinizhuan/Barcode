#include "test_precomp.hpp"

TEST(basic_test, build_objects) {
    using namespace cv;
    BarcodeDetector bardet;
}

TEST(basic_test, get_path) {
    constexpr int buffersize = 80;
    char buf[buffersize]{'\n'};
    getcwd(buf, sizeof(char) * buffersize);
    std::cout << "current working directory" << buf << '\n';
}

TEST(basic_test, BarcodeDetectorPic_1) {
    cv::BarcodeDetector bardet;
    cv::ean_decoder decoder("");
    std::string graphs[] = {
            "ZXing2.png", "1.jpg", "1.png",
            "2.png", "20200325170404705.png",
            "3.jpg", "EAN2.jpg",
            "EAN3.jpg", "EAN.jpg",
            "K71NM_45.jpg",
            "TB20.jpg",
            "utf-8download_45.png", "download.jpg",
            "downloads.webp", "example.jpg",
            "k71n1.png", "multi-barcodes.jpg",
            "slope.jpg", "tail_ean13_2.jpg"
    };
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