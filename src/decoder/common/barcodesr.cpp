
#include "barcodesr.hpp"

namespace cv{
namespace barcode{

BarcodeSR::~BarcodeSR() = default;

BarcodeSR::BarcodeSR()
{
    BarcodeSR::model_dir = "D:/Project/graduation project/thesis/FSRCNN/FSRCNN_Tensorflow/models/";
    BarcodeSR::model = "FSRCNN_x4.pb";
    BarcodeSR::impl.readModel(model_dir+model);
}

BarcodeSR::BarcodeSR(std::string model_path)
{
    impl.readModel(model_path);
}

Mat BarcodeSR::upsample(Mat src, uint8_t scale)
{
    BarcodeSR::impl.setModel("fsrcnn", scale);
    Mat dst;
    impl.upsample(src, dst);
    return dst;
}
}
}

