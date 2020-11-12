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
            "1.jpg", "1.png",
            "2.png", "20200325170404705.png",
            "3.jpg", "EAN2.jpg",
            "EAN3.jpg", "EAN.jpg",
            "K71NM_45.jpg",
            "TB20.jpg", "ZXing2.png",
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