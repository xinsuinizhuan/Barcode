//
// Created by nanos on 2020/10/21.
//
#include "detector/detect.hpp"

int main(int argc, char **argv) {
    using namespace cv;
    std::cout << __DATE__ << " " << __TIME__ << std::endl;

    if (argc < 2) {
        VideoCapture capture(0);
        capture.set(CAP_PROP_FRAME_WIDTH, 1920);
        capture.set(CAP_PROP_FRAME_HEIGHT, 1080);
        BarcodeDetector bardet;
        Mat frame;
        clock_t start;
        float fps;
        Point2f vertices[4];
        while (true) {
            start = clock();
            capture.read(frame);
            std::vector<RotatedRect> rects;
            bardet.detect(frame, rects);
            ean_decoder decoder("");
            std::vector<std::string> results = decoder.rect_to_ucharlist(frame, rects);
            int i = 0;
            for (auto &rect : rects) {
                rect.points(vertices);
                for (int j = 0; j < 4; j++)
                    line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0));
                cv::putText(frame,results[i],vertices[2],cv::FONT_HERSHEY_PLAIN,1,Scalar(255,0,0),2);
                i++;
            }
            fps = 1.0f * CLOCKS_PER_SEC / (float) (clock() - start);
            std::cout << fps << " fps" << std::endl;
            imshow("bounding boxes", frame);
//            for(int i = 0;i<bardet.rotated)
            if (waitKey(1) > 0) break;


        }
    }
    return 0;
}