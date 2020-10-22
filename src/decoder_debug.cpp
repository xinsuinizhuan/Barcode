//
// Created by nanos on 2020/10/21.
//
#include "detector/detect.hpp"

int main(int argc, char **argv) {
    using namespace cv;
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    if (argc >= 2) {
        string path = argv[1];
        for (int i = 0; i < 15; ++i) {
            int number = 20 * i;
            Mat frame = cv::imread(path);
            {
                // rorate
                Size dst_sz(frame.cols, frame.rows);
                cv::Point2f center(static_cast<float>(frame.cols / 2), static_cast<float>(frame.rows / 2));
                Mat rorate = cv::getRotationMatrix2D(center, number, 1.0);
                cv::warpAffine(frame, frame, rorate, dst_sz, cv::INTER_LINEAR, cv::BORDER_REPLICATE);
            }
            BarcodeDetector bd;
            vector<RotatedRect> vec_rate;
            vector<string> decode_resutls;
            bd.detect(frame,  vec_rate);
            bd.decode(frame,vec_rate,decode_resutls);
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
            for (const auto &item : decode_resutls) {
                std::cout << item << std::endl;
//                Point2f vertices[4];
            }
            for (auto &rect : vec_rate) {
                rect.points(vertices);
                for (int j = 0; j < 4; j++) {
                    line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0));
                }
                line(frame, begin, end, Scalar(255, 0, 0));

            }
            imshow("origin", frame);
            //resize(frame, frame, {frame.size().width >> 1, frame.size().height >> 1},0,0,INTER_AREA);
            // imshow("bounding boxes", frame);
            // imshow("gray graph", grayframe);
            waitKey();
        }

    }
    return 0;
}