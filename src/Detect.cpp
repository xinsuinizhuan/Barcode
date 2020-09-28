#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include "vector"
#include "string"

using namespace cv;

class Detect {
private:
    Mat srcImage;//源图像
    Mat dstIamge;//转换为的图像

public:
    explicit Detect(const std::string &);

    std::vector<RotatedRect> getRect();

    Mat getSrc();

};

Detect::Detect(const std::string &imgURL) {
    srcImage = imread(imgURL);//"F:\Pictures\qr.png"
    if (srcImage.empty()) {
        // PS: 现在输出中文会有乱码
        std::cout << "file not no exist" << '\n';
        exit(1);
    }
    cvtColor(srcImage, dstIamge, CV_RGBA2GRAY);
    //imshow("灰度图", dstIamge);
    //waitKey(0);
    GaussianBlur(dstIamge, dstIamge, Size(3, 3), 0);
    //imshow("高斯模糊图", dstIamge);
    //waitKey(0);
    Mat imageSobelX, imageSobelY;
    Sobel(dstIamge, imageSobelX, CV_16S, 1, 0);
    Sobel(dstIamge, imageSobelY, CV_16S, 0, 1);
    convertScaleAbs(imageSobelX, imageSobelX, 1, 0);
    convertScaleAbs(imageSobelY, imageSobelY, 1, 0);

    Mat horizontalImage = imageSobelX - imageSobelY;
    Mat verticalImage = imageSobelY - imageSobelX;
    imshow("x-y", horizontalImage);
    imshow("y-x", verticalImage);

    //waitKey(0);
    Mat horizontalElement = getStructuringElement(MORPH_CROSS, Size(5, 1));
    Mat verticalElement = getStructuringElement(MORPH_CROSS, Size(1, 5));
    Mat element = getStructuringElement(MORPH_CROSS, Size(10, 10));

    auto process = [horizontalElement, verticalElement, element](Mat img, bool direction) {
        blur(img, img, Size(3, 3));
        medianBlur(img, img, 3);
        //imshow("均值滤波图", dstIamge);
        //waitKey(0);

        threshold(img, img, 80, 255, CV_THRESH_BINARY);
        //imshow("二值化图", dstIamge);
        //waitKey(0);
        if (direction) {
            //先在水平方向上膨胀，填充条码中间的空隙
            morphologyEx(img, img, MORPH_DILATE, horizontalElement);

            //在垂直方向上腐蚀，分离条码和字符
            morphologyEx(img, img, MORPH_ERODE, verticalElement);
        } else {
            morphologyEx(img, img, MORPH_DILATE, verticalElement);
            morphologyEx(img, img, MORPH_ERODE, horizontalElement);


        }
        morphologyEx(img, img, MORPH_OPEN, element);
        morphologyEx(img, img, MORPH_CLOSE, element);

    };

    process(verticalImage, false);
    process(horizontalImage, true);

    //resize(image, image, Size(srcimage.cols, srcimage.rows), 0, 0, INTER_AREA);

    dstIamge = verticalImage + horizontalImage;
    //去除字符


    erode(dstIamge, dstIamge, element);
    erode(dstIamge, dstIamge, element);
    dilate(dstIamge, dstIamge, element);

    imshow("去除间隙", dstIamge);
    waitKey();
}

Mat Detect::getSrc() {
    return srcImage;
}

std::vector<RotatedRect> Detect::getRect() {
    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hiera;
    Mat cimage;
    findContours(dstIamge, contours, hiera, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    std::vector<RotatedRect> barcodes;
    for (auto &contour : contours) {
        double area = contourArea(contour);
        RotatedRect rect = minAreaRect(contour);
        double ratio = area / (rect.size.width * rect.size.height);
        if (ratio > 0.75 && area > 200) {
            if (rect.size.width < rect.size.height) {
                rect.angle -= 90.;
                swap(rect.size.width, rect.size.height);
            }
            rect.size.width *= 1.1;
            //drawContours(srcImage, contours, i, 255, -1);
            barcodes.push_back(rect);
        }
    }
    //imshow("框框图", srcImage);
    //waitKey();
    return barcodes;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::string path;
        std::cin >> path;
        auto *myclass = new Detect(path);
        std::vector<RotatedRect> barcodes = myclass->getRect();
        for (auto &barcode : barcodes) {

            //printf("%f %f\n", barcodes[n].size.width, barcodes[n].size.height);

            Point2f vertices[4];
            barcode.points(vertices);
            Point2f dst_vertices[] = {
                    Point2f(0, barcode.size.height - 1),
                    Point2f(0, 0),
                    Point2f(barcode.size.width - 1, 0),
                    Point2f(barcode.size.width - 1, barcode.size.height - 1)};

            Mat M = getPerspectiveTransform(vertices, dst_vertices);
            Mat perspective;
            warpPerspective(myclass->getSrc(), perspective, M, barcode.size, cv::INTER_LINEAR);
            imshow("框框图", perspective);
            waitKey();

            for (int i = 0; i < 4; i++)
                line(myclass->getSrc(), vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
        }
        imshow("框框图", myclass->getSrc());
        waitKey();

    }
    return 0;
}