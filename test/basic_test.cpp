#include "test_precomp.hpp"

std::string ean13_graphs[] = {
        "1_normal.jpg", "2_normal.png",
        "3_normal.jpg", "4_normal.png",
        "5_corner_small.jpg", "6_vertical_corner_small.jpg",
        "7_tilt45.png", "8_tilt330.jpg", "9_dirty_tilt30.jpg",
        "10_dirty_graph.jpg", "11_tilt45.png",
        "12_middle.png",
};
std::string public_graphs[] = {
        "1_detect.png", "2_tilt.jpg", "3_tilt.webp",
        "4_tilt45.jpg", "5_many.jpg", "6_multi_barcodes.jpg"
};
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


TEST(basic_test, Detects) {
    for (const auto &item : public_graphs) {
        const std::string &name_current_image = item;
        const std::string path = R"(./../../test/data/)";
        cv::Mat frame = cv::imread(path + name_current_image);
        ASSERT_FALSE(frame.empty()) << "Can't read image: " << name_current_image;

        cv::BarcodeDetector barcodeDetector;
        cv::Point2f points[4];
        std::vector<cv::RotatedRect> rects;
        try {
            barcodeDetector.detect(frame, rects);
        }
        catch (cv::Exception &ex) {
            std::cerr << ex.what() << "No detect pictures\n";
        }
        for (const auto &rect : rects) {
            rect.points(points);
            for (int j = 0; j < 4; j++) {
#ifdef CV_DEBUG
                cv::line(frame, points[j % 4], points[(j + 1) % 4], cv::Scalar(0, 255, 0));
#endif
            }
        }
        std::cout << name_current_image << " " << rects.size() << std::endl;
        EXPECT_GT(rects.size(), 0);
#ifdef CV_DEBUG
        cv::imshow(name_current_image, frame);
        cv::waitKey(0);
#endif
    }
}


TEST(basic_test, Ean13Decodes) {
    cv::BarcodeDetector barcodeDetector;
    cv::Point2f points[4];
    auto count = 0;
    for (const auto &i: ean13_graphs) {

        std::cout << i << ':' << std::endl;
        cv::Mat frame = cv::imread(R"(./../../test/ean13/)" + i);
        std::vector<cv::RotatedRect> rects;
        try {
            barcodeDetector.detect(frame, rects);
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
            barcodeDetector.decode(frame, rects, results);
        }
        catch (cv::Exception &ex) {
            std::cerr << std::string(2, ' ') << ex.what() << "No detect results\n";
            continue;
        }
        bool judge = false;
        int exist_number = 0;
        for (const auto &result : results) {
            if (result.size() == 13 && isdigit(result[0])) {
                std::cout << result << std::endl;
                exist_number++;
                judge = true;
            }
        }
        std::cout << std::string(2, ' ') << results.size() << " exists " << exist_number << " Barcodes" << std::endl;
        count += judge;
#ifdef CV_DEBUG
        cv::imshow(i, frame);
        cv::waitKey(0);
#endif
    }
    std::cout << std::end(ean13_graphs) - std::begin(ean13_graphs) << " " << count << std::endl;

}


TEST(basic_test, BinaryzationTest) {
    cv::Mat image = imread("./../../test/data/real.jpg",cv::IMREAD_GRAYSCALE);
    imshow("img",image);
    cv::Mat dst = image.clone();
    equalizeHist(image, dst);
    cv::medianBlur(dst,dst,3);
    imshow("dst", dst);

    cv::Mat adapt = dst.clone();
    cv::Mat ostu;
    cv::threshold(adapt,ostu,0,255,cv::THRESH_BINARY|cv::THRESH_OTSU);
    imshow("ostu", ostu);

    //局部二值化
    cv::Mat local_img;
    adaptiveThreshold(adapt, local_img, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 9, 1);
    imshow("local",local_img);

    cv::Mat adapt_img;
    adaptBinaryzation(adapt, adapt_img);
    imshow("adapt", adapt_img);



    cv::Mat adapt_normal;
    resize(adapt_img,adapt_normal,cv::Size(local_img.cols, local_img.rows));
    imshow("resizeAdapt", adapt_normal);

    cv::waitKey();
}
TEST(basic_test, ImgUnitTest) {
    std::string img_path = "./../../test/data/real.jpg";
    cv::BarcodeDetector bardet;
    cv::Mat frame = cv::imread(img_path,cv::IMREAD_GRAYSCALE);
    cv::Mat decodeFrame = frame.clone();
    std::vector<cv::RotatedRect> rects;
    cv::Point2f points[4];
    try {
        bardet.detect(frame, rects);
    }
    catch (cv::Exception &ex) {
        std::cerr << ex.what() << "No detect pictures\n";
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
        bardet.decode(decodeFrame, rects, results);
    }
    catch (cv::Exception &ex) {
        std::cerr << ex.what() << "No detect results\n";
    }
    for (const auto &result : results) {
        std::cout << result << std::endl;
    }
#ifdef CV_DEBUG
    cv::imshow("result", frame);
    cv::imshow("resultdecode", decodeFrame);
    cv::waitKey(0);
#endif
}


TEST(basic_test, RotateTest) {
    cv::Mat image = imread("./../../test/data/real2.jpg",cv::IMREAD_GRAYSCALE);
    cv::Mat test;
    cv::RotatedRect rect(cv::Point2f(134,91),cv::Size(141,293),-89);
    cv::Point2f pts[4];
    rect.points(pts);
    for (int i = 0; i < 4; i++) {
        std::cout << "draw" << std::endl;
        cv::line(image, pts[i], pts[(i + 1) % 4], cv::Scalar(0, 0, 0));
    }

    cv::cutImage(image, test, rect);
    cv::imshow("origin", test);
    equalizeHist(test, test);
    cv::adaptBinaryzation(test, test);

    cv::imshow("test", test);
    cv::imshow("img", image);
    cv::waitKey();
}