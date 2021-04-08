#include "test_precomp.hpp"
#include "verifier.hpp"

#include <opencv2/highgui.hpp>

std::string enumToString(cv::barcode::BarcodeType format)
{
    using namespace cv::barcode;
    switch (format)
    {
        case BarcodeType::EAN_13:
            return "EAN_13";
        case BarcodeType::EAN_8:
            return "EAN_8";
        case BarcodeType::UPC_E:
            return "UPC_E";
        default:
            return "NONE";
    }
}

TEST(integration_testing, detect_and_decode)
{
    //change model path
    std::string dir = "../../model/";
    std::string model = "sr.caffemodel";
    std::string prototxt = "sr.prototxt";

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
    Verifier verifier{dir + prototxt, dir + model, data_path, result_file, img_types};
    cv::TickMeter time;
    time.start();
    verifier.verify();
    time.stop();
    std::cout << "Error detection num: " << verifier.error_detection_num << std::endl;
    std::cout << "time: " << time.getTimeSec() << " s" << std::endl;
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
