//
// Created by nanos on 2020/10/21.
//
#include "barcode.hpp"
#include <direct.h>
void showInfo(cv::Mat frame, std::vector<std::string> infos)
{
    //Prompt
    infos.push_back("type \'s\' to capture screenshot");
    cv::Point2f start(5, 10);
    cv::Point2f step(0, 10);

    for (std::string info : infos)
    {
        cv::putText(frame, info, start, cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1);
        start += step;
    }
}


int main(int argc, char **argv)
{
    using namespace cv;
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    char *buffer;
    buffer = getcwd(NULL, 0);
    std::cout << buffer << std::endl;
    if (argc < 2)
    {
        printf("Usage: ./main_debug --webcam\n"
               "./main_debug <input_file>\n");
        exit(-1);
    }
    BarcodeDetector bardet;
    ean_decoder ean13_decoder{EAN::TYPE13};
    Mat frame;
    Point2f vertices[4];
    clock_t start;
    std::vector<RotatedRect> rects;
    std::vector<string> decoded_info;
    string fps;
    bool ok;

    std::string right_result;
    std::string test_dir = "../../test/";
    std::string postfix = ".jpg";
    std::vector<string> wrong_results;
    int total_cnt = 0;
    bool has_result = false;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-r") == 0 && i + 1 < argc)
        {
            right_result = argv[i + 1];
            has_result = true;
        }
    }
    std::map<std::string, bool> img_map;

    if (strcmp(argv[1], "--webcam") == 0)
    {
        VideoCapture capture(0);
        capture.set(CAP_PROP_FRAME_WIDTH, 1280);
        capture.set(CAP_PROP_FRAME_HEIGHT, 720);

//        float fps;
        while (true)
        {
            start = clock();
            capture.read(frame);
            Mat frame_copy = frame.clone();
            ok = bardet.detectAndDecode(frame, decoded_info, rects);
            if (ok)
            {
                for (auto &info:decoded_info)
                {
                    std::cout << info << std::endl;
                }
                int i = 0;
                for (auto &rect : rects)
                {
                    if (has_result && decoded_info[i].length() == 13 && right_result != decoded_info[i])
                    {
                        if (img_map.find(decoded_info[i]) == img_map.end())
                        {
                            std::string dir = test_dir + "wrong_decode/";
                            img_map[decoded_info[i]] = true;
                            imwrite(dir + decoded_info[i] + postfix, frame);
                            wrong_results.push_back(decoded_info[i]);
                        }
                    }
                    rect.points(vertices);
                    cv::putText(frame, decoded_info[i], vertices[2], cv::FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0), 2);
                    if (decoded_info[i] == "ERROR")
                    {
                        for (int j = 0; j < 4; j++)
                            line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 0, 255), 2);
                    }
                    else
                    {
                        for (int j = 0; j < 4; j++)
                            line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0), 2);
                    }
                    i++;
                }
            }

            fps = "FPS: " + std::to_string(CLOCKS_PER_SEC / (clock() - start));
//            std::cout << fps << " fps" << std::endl;
            //cv::putText(frame, fps, Point2f(5, 10), cv::FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1);
            std::vector<string> infos;
            infos.push_back(fps);
            if (has_result)
            {
                infos.push_back("right result: " + right_result);
                for (auto info:wrong_results)
                {
                    infos.push_back("wrong result: " + info);
                }

            }

            int key = waitKey(1);
            if (key == 's')
            {
                time_t t = time(0);
                char *dt = ctime(&t);
                std::string date(dt);
                date.erase(remove(date.begin(), date.end(), '\n'), date.end());
                date.erase(remove(date.begin(), date.end(), ' '), date.end());
                replace(date.begin(), date.end(), ':', '_');
                std::string path = test_dir + "screenshot/";

                if(imwrite(path+date+postfix, frame_copy))
                    std::cout << "save success " << path+date+postfix << std::endl;
                else
                    std::cout << "fail" << path+date+postfix << std::endl;
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
        frame = imread(argv[1]);
        start = clock();

        ok = bardet.detectAndDecode(frame, decoded_info, rects);
        if (ok)
        {
            for (auto &info:decoded_info)
            {
                std::cout << info << std::endl;
            }
            int i = 0;
            for (auto &rect : rects)
            {
                rect.points(vertices);
                cv::putText(frame, decoded_info[i], vertices[2], cv::FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0), 2);
                if (decoded_info[i] == "ERROR")
                {
                    for (int j = 0; j < 4; j++)
                        line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 0, 255), 2);
                }
                else
                {
                    for (int j = 0; j < 4; j++)
                        line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0), 2);
                }
                i++;
            }
        }


        fps = std::to_string(clock() - start) + " ms";
        cv::putText(frame, fps, Point2f(5, 10), cv::FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1);
        std::cout << fps << std::endl;
        imshow("bounding boxes", frame);

        waitKey();
    }
    return 0;

}