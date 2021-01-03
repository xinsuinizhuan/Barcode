#include "test_precomp.hpp"
#include "verifier.hpp"
#ifdef CV_DEBUG

#include <opencv2/highgui.hpp>

#endif
std::string enumToString(cv::barcode::BarcodeFormat format)
{
    using namespace cv::barcode;
    switch(format)
    {
        case BarcodeFormat::EAN_13:
            return "EAN_13";
        case BarcodeFormat::EAN_8:
            return "EAN_8";
        case BarcodeFormat::UPC_E:
            return "UPC_E";
        default:
            return "NONE";
    }
}

TEST(integration_testing, detect_and_decode)
{
    std::string pre_path = R"(./../../)";
    std::ifstream correctness_file;
    float last_correctness = 0;
    std::string path{pre_path + "test/data/integration_test_data/correctness.txt"};
    correctness_file.open(path);
    if (correctness_file.is_open())
    {
        correctness_file >> last_correctness;
        correctness_file.close();
    }

    //图->答案, 图list,读取文件中所有的图ignore .csv
    std::vector<std::string> img_types = {"jpg", "png"};
    std::string data_path{pre_path + "test/data/integration_test_data/"};
    std::string result_file{pre_path + "test/data/integration_test_data/result.csv"};
    Verifier verifier{data_path, result_file, img_types};
    verifier.verify();
    float correctness = verifier.getCorrectness();
    if (correctness >= last_correctness - 0.00001)
    {
        std::cout << "pass rate: " << correctness * 100 << "%" << "\n";
        std::cout << "last: " << last_correctness * 100 << "%" << std::endl;
        std::ofstream out_correctness_file;
        out_correctness_file.open(path);
        if (out_correctness_file.is_open())
        {
            out_correctness_file << correctness;
            out_correctness_file.close();
        }
    }
    else
    {
        std::cout << "pass rate is lesser than last time!" << std::endl;
        std::cout << "last: " << last_correctness << ", now: " << correctness << std::endl;
    }
    ASSERT_TRUE(correctness >= last_correctness - 0.00001);
}

TEST(integration_testing, ImgUnitTest)
{
    std::string img_path = R"(../../test/data/integration_test_data/1.jpg)";
    cv::barcode::BarcodeDetector bardet;
    cv::Mat frame = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
    cv::Mat decodeFrame = frame.clone();
    std::vector<cv::RotatedRect> rects;
    std::vector<cv::Point2f> points;
    std::vector<std::string> barcode_info;
    std::vector<cv::barcode::BarcodeFormat> barcode_format;
    try
    {
        bardet.detectAndDecode(frame, barcode_info,barcode_format , points);
    } catch (cv::Exception &ex)
    {
        std::cerr << ex.what() << "No detect pictures\n";
    }
    for (int i = 0;i < barcode_info.size();i ++)
    {
        std::cout << barcode_info[i] << ", format: " << enumToString(barcode_format[i]) << std::endl;
    }
#ifdef CV_DEBUG
    cv::imshow("result", frame);
    cv::imshow("resultdecode", decodeFrame);
    cv::waitKey(0);
#endif
}

