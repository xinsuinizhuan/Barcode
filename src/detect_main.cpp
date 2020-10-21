//
// Created by nanos on 2020/10/21.
//
#include "detector/detect.hpp"
#include "decoder/ean_decoder.hpp"

int main(int argc, char **argv) {
    using namespace cv;
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    bool test_functions = true;
    if (test_functions) {
        Mat frame = cv::imread(R"(../test/data/3.jpg)");
        Mat grayframe = cv::imread(R"(../test/data/2.png)", IMREAD_GRAYSCALE);
        Mat binary;
        cv::threshold(grayframe, binary, 5, 255, THRESH_BINARY | THRESH_OTSU);
        BarcodeDetector bd;
        vector<RotatedRect> vec_rate;
        bd.detect(frame, vec_rate);
        ean_decoder decoder("");


        // TODO refactor in here, it seems that it need a binary-rafactored matrix to decode.
//        int height = binary.rows;
//        int center = height / 2;
//        cv::Mat centerLine = binary.rowRange(center, center + 1);
//        std::vector<uchar> middle_array = centerLine.isContinuous() ? centerLine : centerLine.clone();
//        std::cout << decoder.decode(std::vector<uchar>(middle_array.rbegin(),middle_array.rend()), 0) << std::endl;
        Point2f begin;
        Point2f end;
        Point2f vertices[4];
        vec_rate[0].points(vertices);

        double distance1 = cv::norm(vertices[0] - vertices[1]);
        double distance2 = cv::norm(vertices[1] - vertices[2]);
        if (distance1 > distance2) {
            begin = (vertices[0] + vertices[3]) / 2;
            end = (vertices[1] + vertices[2]) / 2;
        } else {
            begin = (vertices[0] + vertices[1]) / 2;
            end = (vertices[2] + vertices[3]) / 2;
        }

        for (const auto &i: decoder.rect_to_ucharlist(grayframe, vec_rate)) {
            std::cout << i << std::endl;
            Point2f vertices[4];
            for (auto &rect : vec_rate) {
                rect.points(vertices);
                for (int j = 0; j < 4; j++) {
                    line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0));
                }

                //cv::putText(frame, "begin", (vertices[0] + vertices[1]) / 2, cv::FONT_HERSHEY_PLAIN, 2, 0x0);
                //resize(frame, frame, {frame.size().width >> 1, frame.size().height >> 1},0,0,INTER_AREA);
                line(frame,begin,end,Scalar(255,0,0));
                imshow("origin", frame);
                //imshow("bounding boxes", frame);
                //imshow("binary graph", binary);
                //imshow("gray graph", grayframe);
                waitKey();
            }
        }
        exit(-1);
    }
//    if (argc < 2) {
//        std::cout << "need a VideoCapture" << std::endl;
//        VideoCapture capture(0);
//        capture.set(CAP_PROP_FRAME_WIDTH, 1920);
//        capture.set(CAP_PROP_FRAME_HEIGHT, 1080);
//        Detect bardet;
//        Mat frame;
//        clock_t start;
//        float fps;
//        while (true) {
//            start = clock();
//            capture.read(frame);
//            bardet.init(frame);
//            bardet.localization(true);
//
//            imshow("bounding boxes", bardet.getCandidatePicture());
//            fps = 1.0f * CLOCKS_PER_SEC / (float) (clock() - start);
//            std::cout << fps << " fps" << std::endl;
////            for(int i = 0;i<bardet.rotated)
//            if (waitKey(1) > 0) break;
//
//
//        }
//    }
    return 0;
}