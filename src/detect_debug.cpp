//
// Created by nanos on 2020/10/21.
//
#include "detector/detect.hpp"
#include "decoder/ean_decoder.hpp"

int main(int argc, char **argv) {
    using namespace cv;
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    if (argc < 2) {
//        VideoCapture capture(0);
//        capture.set(CAP_PROP_FRAME_WIDTH, 1920);
//        capture.set(CAP_PROP_FRAME_HEIGHT, 1080);
        BarcodeDetector bardet;
        Mat frame;
        Point2f vertices[4];
        frame = imread("../test/data/1.png");
        std::vector<RotatedRect> rects;
        bardet.detect(frame, rects, true);

        ean_decoder decoder("EAN13");
        std::vector<std::string> results = decoder.rect_to_ucharlist(frame,rects);

        //draw
        int i = 0;
        for (auto &rect : rects) {
            rect.points(vertices);
            for (int j = 0; j < 4; j++)
                line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0), 2);
            cv::putText(frame,results[i],vertices[2],cv::FONT_HERSHEY_PLAIN,1,Scalar(255,0,0),2);
            i++;
        }
        imshow("bounding boxes", frame);
        waitKey();

    }
    return 0;
}