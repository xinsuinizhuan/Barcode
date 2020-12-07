#include "test_precomp.hpp"

TEST(binaryzation_test, BinaryzationTest)
{
    cv::Mat image = imread("../../test/data/real.jpg", cv::IMREAD_GRAYSCALE);
    imshow("img", image);
    cv::Mat dst = image.clone();
    //pre process
    equalizeHist(image, dst);
    cv::medianBlur(dst, dst, 3);
    imshow("dst", dst);

    cv::Mat ostu;
    cv::threshold(dst, ostu, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    imshow("ostu", ostu);

    //局部二值化
    cv::Mat local_img;
    adaptiveThreshold(dst, local_img, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 9, 1);
    imshow("local", local_img);

    cv::Mat adapt_img;
    adaptBinaryzation(dst, adapt_img);
    imshow("gradientBinaryzation", adapt_img);

    enhanceLocalBinaryzation(dst, dst, 5, 0.9);
    imshow("enhance", dst);
    cv::waitKey();
}

TEST(binaryzation_test, ImgUnitTest)
{
    std::string img_path = R"(../../test/data/integration_test_data/4.jpg)";
    cv::BarcodeDetector bardet;
    cv::Mat frame = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
    cv::Mat decodeFrame = frame.clone();
    std::vector<cv::RotatedRect> rects;
    cv::Point2f points[4];
    try
    {
        bardet.detect(frame, rects);
    } catch (cv::Exception &ex)
    {
        std::cerr << ex.what() << "No detect pictures\n";
    }
    for (const auto &rect : rects)
    {
        rect.points(points);
        for (int j = 0; j < 4; j++)
        {
#ifdef CV_DEBUG
            cv::line(frame, points[j % 4], points[(j + 1) % 4], cv::Scalar(0, 255, 0));
#endif
        }
    }
    std::vector<std::string> results;
    try
    {
        bardet.decode(decodeFrame, rects, results);
    } catch (cv::Exception &ex)
    {
        std::cerr << ex.what() << "No detect results\n";
    }
    for (const auto &result : results)
    {
        std::cout << result << std::endl;
    }
#ifdef CV_DEBUG
    cv::imshow("result", frame);
    cv::imshow("resultdecode", decodeFrame);
    cv::waitKey(0);
#endif
}


TEST(binaryzation_test, RotateTest)
{
    cv::Mat image = imread("./../../test/data/real2.jpg", cv::IMREAD_GRAYSCALE);
    cv::Mat test;
    cv::RotatedRect rect(cv::Point2f(134, 200), cv::Size(141, 293), -30);
    cv::Point2f pts[4];
    rect.points(pts);
    for (int i = 0; i < 4; i++)
    {
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

