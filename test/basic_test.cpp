#include "test_precomp.hpp"

std::string ean13_graphs[] = {
        "1_normal.jpg", "2_normal.png", "3_normal.jpg", "4_normal.png", "5_corner_small.jpg"
        , "6_vertical_corner_small.jpg", "7_tilt45.png", "8_tilt330.jpg", "9_dirty_tilt30.jpg", "10_dirty_graph.jpg"
        , "11_tilt45.png", "12_middle.png",};
std::string public_graphs[] = {
        "1_detect.png", "2_tilt.jpg", "3_tilt.webp", "4_tilt45.jpg", "5_many.jpg", "6_multi_barcodes.jpg"};
TEST(basic_test, build_objects)
{
    using namespace cv;
    BarcodeDetector bardet;
}

TEST(basic_test, get_path)
{
    constexpr int buffersize = 80;
    char buf[buffersize]{'\n'};
    getcwd(buf, sizeof(char) * buffersize);
    std::cout << "current working directory" << buf << '\n';
}


TEST(basic_test, Detects)
{
    for (const auto &item : public_graphs)
    {
        const std::string &name_current_image = item;
        const std::string path = R"(./../../test/data/)";
        cv::Mat frame = cv::imread(path + name_current_image);
        ASSERT_FALSE(frame.empty()) << "Can't read image: " << name_current_image;
        
        cv::BarcodeDetector barcodeDetector;
        cv::Point2f points[4];
        std::vector<cv::RotatedRect> rects;
        try
        {
            barcodeDetector.detect(frame, rects);
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
        std::cout << name_current_image << " " << rects.size() << std::endl;
        EXPECT_GT(rects.size(), 0);
#ifdef CV_DEBUG
        cv::imshow(name_current_image, frame);
        cv::waitKey(0);
#endif
    }
}


TEST(basic_test, Ean13Decodes)
{
    cv::BarcodeDetector barcodeDetector;
    cv::Point2f points[4];
    auto count = 0;
    for (const auto &i: ean13_graphs)
    {
        std::cout << i << ':' << std::endl;
        cv::Mat frame = cv::imread(R"(./../test/ean13/)" + i);
        std::vector<cv::RotatedRect> rects;
        try
        {
            barcodeDetector.detect(frame, rects);
        } catch (cv::Exception &ex)
        {
            std::cerr << ex.what() << "No detect pictures\n";
            continue;
        }
        std::vector<std::string> results;
        try
        {
            barcodeDetector.decode(frame, rects, results);
        } catch (cv::Exception &ex)
        {
            std::cerr << std::string(2, ' ') << ex.what() << "No detect results\n";
            continue;
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
        bool judge = false;
        int exist_number = 0;
        for (const auto &result : results)
        {
            if (result.size() == 13 && isdigit(result[0]))
            {
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


TEST(basic_test, isValidEan13)
{
    int methodLength = 13;
    auto test_function = [&methodLength](std::string result) {
        if (result.size() != methodLength)
        {
            return false;
        }
        int sum = 0;
        for (int index = result.size() - 2, i = 1; index >= 0; index--, i++)
        {
            int temp = result[index] - '0';
            sum += (temp + ((i & 1) != 0 ? temp << 1 : 0));
        }
        return (result.back() - '0') == (10 - (sum % 10)) % 10;
    };
    std::string correct_array[] = {"4010948996163", "9780201379624", "2020111711167", "8750130032670", "5231002444232"};
    for (const auto &item : correct_array)
    {
        EXPECT_TRUE(test_function(item));
        std::string copyed = item;
        for (int i = 0; i < 10; ++i)
        {
            copyed.back() = '0' + i;
            if (i != item.back() - '0')
            {
                EXPECT_FALSE(test_function(copyed));
            }
            else
            {
                EXPECT_TRUE(test_function(copyed));
            }
        }
    }
}