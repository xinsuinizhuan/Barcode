//
// Created by nanos on 2020/10/21.
//
#include "opencv2/barcode.hpp"
#include <opencv2/opencv.hpp>
#include <direct.h>

using namespace cv;

void showInfo(Mat frame, std::vector<std::string> infos)
{
    //Prompt
    infos.emplace_back("type \'s\' to capture screenshot");
    Point2f start(5, 10);
    Point2f step(0, 10);

    for (const std::string &info : infos)
    {
        putText(frame, info, start, cv::FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1);
        start += step;
    }
}


int main(int argc, char **argv)
{
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    char *buffer;
    buffer = _getcwd(nullptr, 0);
    std::cout << buffer << std::endl;

    const std::string keys = "{h help ?     |        | print help messages }"
                             "{i in         |        | input image path (also switches to image detection mode) }"
                             "{r right      |        | compare the right result }"
                             "{v video_idx  |  0     | capture video index }";
    CommandLineParser cmd_parser(argc, argv, keys);

    cmd_parser.about("This program detects the bar-codes from camera or images using the OpenCV library.");
    if (cmd_parser.has("help"))
    {
        cmd_parser.printMessage();
        return 0;
    }
    barcode::BarcodeDetector bardet("sr.prototxt", "sr.caffemodel");
    Mat frame;
    clock_t start;
    std::vector<Point2f> points;
    std::vector<std::string> decoded_info;
    std::vector<cv::barcode::BarcodeType> decoded_format;
    std::string fps;
    bool ok;

    std::string test_dir = "../../test/";
    std::string postfix = ".jpg";
    std::vector<std::string> wrong_results;
//    int total_cnt = 0;
    std::string in_file_name = cmd_parser.get<std::string>("in");
    std::string right_result = cmd_parser.get<std::string>("right");
    int video_idx = cmd_parser.get<int>("video_idx");
    if (!cmd_parser.check())
    {
        cmd_parser.printErrors();
        return -1;
    }
    bool has_result = !right_result.empty();

    std::map<std::string, bool> img_map;

    if (in_file_name.empty())
    {

        video_idx = max(0, video_idx);
        VideoCapture capture(video_idx);
        capture.set(CAP_PROP_FRAME_WIDTH, 1280);
        capture.set(CAP_PROP_FRAME_HEIGHT, 720);

//        float fps;
        while (true)
        {
            start = clock();
            capture.read(frame);
            Mat frame_copy = frame.clone();
            ok = bardet.detectAndDecode(frame, decoded_info, decoded_format, points);
            if (ok)
            {
                for (size_t i = 0; i < points.size(); i += 4)
                {
                    size_t bar_idx = i / 4;
                    std::vector<Point> barcode_contour(points.begin() + i, points.begin() + i + 4);
                    std::cout << decoded_info[bar_idx] << " " << decoded_format[bar_idx] << std::endl;

                    if (has_result && decoded_info[bar_idx].length() == 13 && right_result != decoded_info[bar_idx])
                    {
                        if (img_map.find(decoded_info[bar_idx]) == img_map.end())
                        {
                            std::string dir = test_dir + "wrong_decode/";
                            dir.append(decoded_info[bar_idx]).append(postfix);
                            img_map[decoded_info[bar_idx]] = true;
                            imwrite(dir, frame);
                            wrong_results.push_back(decoded_info[bar_idx]);
                        }
                    }
                    cv::putText(frame, decoded_info[bar_idx], barcode_contour[2], cv::FONT_HERSHEY_PLAIN, 1,
                                Scalar(255, 0, 0), 2);
                    if (decoded_format[bar_idx] == barcode::BarcodeType::NONE)
                    {
                        for (int j = 0; j < 4; j++)
                        {
                            line(frame, barcode_contour[j], barcode_contour[(j + 1) % 4], Scalar(0, 0, 255), 2);
                        }
                    }
                    else
                    {
                        for (int j = 0; j < 4; j++)
                        {
                            line(frame, barcode_contour[j], barcode_contour[(j + 1) % 4], Scalar(0, 255, 0), 2);
                        }
                    }
                }


            }

            fps = "FPS: " + std::to_string(CLOCKS_PER_SEC / (clock() - start));
//            std::cout << fps << " fps" << std::endl;
            std::vector<std::string> infos;
            infos.push_back(fps);
            if (has_result)
            {
                infos.push_back("right result: " + right_result);
                for (const auto &info:wrong_results)
                {
                    infos.push_back("wrong result: " + info);
                }

            }

            int key = waitKey(1);
            if (key == 's')
            {
                time_t t = time(nullptr);
                char *dt = ctime(&t);
                std::string date(dt);
                date.erase(remove(date.begin(), date.end(), '\n'), date.end());
                date.erase(remove(date.begin(), date.end(), ' '), date.end());
                replace(date.begin(), date.end(), ':', '_');
                std::string path = test_dir + "screenshot/";
                path.append(date).append(postfix);
                if (imwrite(path, frame_copy))
                {
                    std::cout << "save success " << path << std::endl;
                }
                else
                {
                    std::cout << "fail" << path << std::endl;
                }
            }
            else if (key > 0)
            {
                break;
            }
            showInfo(frame, infos);
            imshow("bounding boxes", frame);
        }

    }
    else
    {
        frame = imread(in_file_name);
        start = clock();
        std::vector<Point2f> points;
        std::vector<std::string> decoded_info;
        ok = bardet.detectAndDecode(frame, decoded_info, decoded_format, points);
        if (ok)
        {
            for (size_t i = 0; i < points.size(); i += 4)
            {
                size_t bar_idx = i / 4;
                std::vector<Point> barcode_contour(points.begin() + i, points.begin() + i + 4);
                std::cout << decoded_info[bar_idx] << " " << decoded_format[bar_idx] << std::endl;

                cv::putText(frame, decoded_info[bar_idx], barcode_contour[1], cv::FONT_HERSHEY_PLAIN, 1,
                            Scalar(255, 0, 0), 2);
                if (decoded_format[bar_idx] == barcode::BarcodeType::NONE)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        line(frame, barcode_contour[j], barcode_contour[(j + 1) % 4], Scalar(0, 0, 255), 2);
                    }
                }
                else
                {
                    for (int j = 0; j < 4; j++)
                    {
                        line(frame, barcode_contour[j], barcode_contour[(j + 1) % 4], Scalar(0, 255, 0), 2);
                    }
                }
            }


        }
        fps = std::to_string(clock() - start) + " ms";
        cv::putText(frame, fps, Point2f(5, 10), cv::FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1);
        std::cout << fps << std::endl;
        imshow("bounding boxes", frame);

        imwrite("./test.png", frame);
        waitKey();
    }
    return 0;

}