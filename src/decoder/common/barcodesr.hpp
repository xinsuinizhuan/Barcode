

#ifndef __OPENCV_BARCODE_BARCODESR_HPP__
#define __OPENCV_BARCODE_BARCODESR_HPP__
#include "opencv2/core/mat.hpp"
#include "opencv2/dnn_superres.hpp"
namespace cv{
namespace barcode{
class BarcodeSR
{
public:
    Mat upsample(Mat src, uint8_t scale);
    BarcodeSR();
    BarcodeSR(std::string model_path);
    ~BarcodeSR();
private:
    std::string model_dir;
    std::string model;
    dnn_superres::DnnSuperResImpl impl;


};

}
}

#endif //__OPENCV_BARCODE_BARCODESR_HPP__
