#include "barcode.hpp"
using namespace cv;


uchar adaptiveProcess(const Mat &im, int row,int col,int kernelSize,int max_size)
{
    vector<uchar> pixels;
    for (int a = -kernelSize / 2; a <= kernelSize / 2; a++)
        for (int b = -kernelSize / 2; b <= kernelSize / 2; b++)
        {
            pixels.push_back(im.at<uchar>(row + a, col + b));
        }
    sort(pixels.begin(), pixels.end());
    auto min = pixels[0];
    auto max = pixels[kernelSize * kernelSize - 1];
    auto med = pixels[kernelSize * kernelSize / 2];
    auto zxy = im.at<uchar>(row, col);
    if (med > min && med < max)
    {
        // to B
        if (zxy > min && zxy < max)
            return zxy;
        else
            return med;
    }
    else
    {
        kernelSize += 2;
        if (kernelSize <= max_size)
            return adaptiveProcess(im, row, col, kernelSize, max_size); // 增大窗口尺寸，继续A过程。
        else
            return med;
    }
}

void adaptiveMeanFilter(Mat src, Mat& dst,int min_size=3, int max_size=7) {

    dst = src.clone();
    // 扩展图像的边界
    copyMakeBorder(src, dst, max_size / 2,  max_size / 2,  max_size / 2,  max_size / 2, BorderTypes::BORDER_REFLECT);
    // 图像循环
    for (int j = max_size / 2; j < dst.rows - max_size / 2; j++)
    {
        for (int i = max_size / 2; i < dst.cols * dst.channels() - max_size / 2; i++)
        {
            dst.at<uchar>(j, i) = adaptiveProcess(dst, j, i, min_size, max_size);
        }
    }
}

void binaryzationTest() {
    Mat image = imread("../test/data/tail_ean13_2.jpg",IMREAD_GRAYSCALE);
    imshow("img",image);
    Mat dst = image.clone();
    equalizeHist(image, dst);
    imshow("hist", dst);

    Mat adapt;
    adaptiveMeanFilter(dst,adapt);
    imshow("adapt img",adapt);

    //局部二值化
    Mat local_img;
    adaptiveThreshold(adapt, local_img, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 7, 0);
    imshow("local",local_img);

    Mat adapt_img;
    adaptBinaryzation(adapt, adapt_img);
    imshow("adapt", adapt_img);

    Mat adapt_normal;
    resize(adapt_img,adapt_normal,Size(local_img.cols, local_img.rows));
    imshow("resizeAdapt", adapt_normal);

    waitKey();
}

void cutImgTest() {
    Mat image = imread("../test/data/test1.png",IMREAD_GRAYSCALE);
    BarcodeDetector barcodeDetector;
    vector<RotatedRect> rects;
    barcodeDetector.detect(image,rects,false);
    vector<Mat> imgs = getBarcodeImgs(image,rects);
    for(int i = 0;i < imgs.size();i ++) {
        std::string title = "img" + i;
        imshow(title,imgs[i]);
    }
    waitKey();
}

int main(int argc, char **argv) {

    binaryzationTest();
}