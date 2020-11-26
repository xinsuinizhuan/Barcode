//
// Created by nanos on 2020/10/21.
//
#include "barcode.hpp"

int main(int argc, char **argv)
{
    using namespace cv;
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    if (argc < 2)
    {
        printf("Usage: ./main_debug --webcam\n"
               "./main_debug <input_file>\n");
        exit(-1);
    }
    std::map<std::string, int> result_cnt;
    double total = 0;
    double max = 0;
    BarcodeDetector bardet;
    Mat frame;
    Point2f vertices[4];
    clock_t start;
    std::vector<RotatedRect> rects;
    std::vector<string> decoded_info;
    std::string right_result;
    std::string dir = "./../test/wrong_decode/";
    std::string postfix = ".jpg";
    bool has_result = false;
    for(int i = 0;i < argc;i ++) {
        if(strcmp(argv[i], "-r") == 0 && i+1 < argc) {
            right_result = argv[i+1];
            has_result = true;
        }
    }
    std::map<std::string, bool> img_map;
    
    if (strcmp(argv[1], "--webcam") == 0)
    {
        VideoCapture capture(0);
        //capture.set(CAP_PROP_FRAME_WIDTH, 1920);
        //capture.set(CAP_PROP_FRAME_HEIGHT, 1080);
        
        float fps;
        while (true)
        {
            start = clock();
            capture.read(frame);
            bardet.detectAndDecode(frame, decoded_info, rects);
            int i = 0;
            for (auto &rect : rects)
            {
                if(has_result && decoded_info[i].length() == 13 && right_result != decoded_info[i]) {
                    if(img_map.find(decoded_info[i]) == img_map.end()) {
                        img_map[decoded_info[i]] = true;
                        imwrite(dir+decoded_info[i]+postfix, frame);
                    }
                }
                rect.points(vertices);
                for (int j = 0; j < 4; j++)
                    line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0), 2);
                cv::putText(frame, decoded_info[i], vertices[2], cv::FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0), 2);
                
                i++;
            }
            fps = 1.0f * CLOCKS_PER_SEC / (float) (clock() - start);
            std::cout << fps << " fps" << std::endl;
            imshow("bounding boxes", frame);
            if (waitKey(1) > 0)
            { break; }
        }
        
    }
    else
    {
        frame = imread(argv[1]);
        bardet.detectAndDecode(frame, decoded_info, rects);
        
        for (auto &info:decoded_info)
        {
            std::cout << info << std::endl;
        }
        int i = 0;
        for (auto &rect : rects)
        {
            rect.points(vertices);
            for (int j = 0; j < 4; j++)
                line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0), 2);
            cv::putText(frame, decoded_info[i], vertices[2], cv::FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0), 2);
            i++;
        }
        imshow("bounding boxes", frame);
        waitKey();
    }
    return 0;
    
}