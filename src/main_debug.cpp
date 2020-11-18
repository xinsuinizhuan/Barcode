//
// Created by nanos on 2020/10/21.
//
#include "barcode.hpp"

int main(int argc, char **argv) {
    using namespace cv;
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    if (argc < 2) {
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

    if (strcmp(argv[1], "--webcam") == 0) {
        VideoCapture capture(0);
        //capture.set(CAP_PROP_FRAME_WIDTH, 1920);
        //capture.set(CAP_PROP_FRAME_HEIGHT, 1080);

        float fps;
        while (true) {
            start = clock();
            capture.read(frame);
            bardet.detectAndDecode(frame, decoded_info, rects);
            decoded_info.push_back(ean13_decoder.decodeDirectly(frame));
            for (auto &info:decoded_info) {
                std::cout << info << std::endl;
            }
            int i = 0;
            for (auto &rect : rects) {
                rect.points(vertices);
                for (int j = 0; j < 4; j++)
                    line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0), 2);
                cv::putText(frame, decoded_info[i], vertices[2], cv::FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0), 2);
                i++;


            }
            fps = 1.0f * CLOCKS_PER_SEC / (float) (clock() - start);
            std::cout << fps << " fps" << std::endl;
            imshow("bounding boxes", frame);
            if (waitKey(1) > 0) break;
        }

    } else {
        frame = imread(argv[1]);
        bardet.detectAndDecode(frame, decoded_info, rects);

        for (auto &info:decoded_info) {
            std::cout << info << std::endl;
        }
        int i = 0;
        for (auto &rect : rects) {
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