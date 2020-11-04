//
// Created by 97659 on 2020/11/3.
//

#include "barcode.hpp"

namespace cv {
    static bool checkBarInputImage(InputArray img, Mat &gray) {
        CV_Assert(!img.empty());
        CV_CheckDepthEQ(img.depth(), CV_8U, "");

        if (img.cols() <= 20 || img.rows() <= 20) {
            return false;  // image data is not enough for providing reliable results
        }
        int incn = img.channels();
        CV_Check(incn, incn == 1 || incn == 3 || incn == 4, "");
        if (incn == 3 || incn == 4) {
            cvtColor(img, gray, COLOR_BGR2GRAY);
        } else {
            gray = img.getMat();
        }
        return true;
    }

    struct BarcodeDetector::Impl {
    public:
        Impl() = default;

        ~Impl() = default;
    };

    BarcodeDetector::BarcodeDetector() : p(new Impl) {}

    BarcodeDetector::~BarcodeDetector() = default;


    bool BarcodeDetector::detect(InputArray img, CV_OUT std::vector<RotatedRect> &rects, bool debug) const {
        Mat inarr;
        if (!checkBarInputImage(img, inarr)) {
            return false;
        }

        Detect bardet;
        bardet.init(inarr);
        bardet.localization(debug);
        vector<RotatedRect> _rects = bardet.getLocalizationRects();
        rects.assign(_rects.begin(), _rects.end());
        return true;
    }

    bool BarcodeDetector::decode(InputArray img, const std::vector<RotatedRect> &rects, CV_OUT
                                 vector<std::string> &decoded_info) const {
        Mat inarr;
        if (!checkBarInputImage(img, inarr)) {
            return false;
        }
        CV_Assert(!rects.empty());
        ean_decoder decoder("");
        vector<std::string> _decoded_info = decoder.rect_to_ucharlist(inarr, rects);
        decoded_info.assign(_decoded_info.begin(), _decoded_info.end());

        return true;
    }

    bool BarcodeDetector::detectAndDecode(InputArray img, CV_OUT vector<std::string> &decoded_info, CV_OUT
                                          vector<RotatedRect> &rects) const {
        Mat inarr;
        if (!checkBarInputImage(img, inarr)) {
            return false;
        }

        Detect bardet;
        bardet.init(inarr);
        bardet.localization();
        vector<RotatedRect> _rects = bardet.getLocalizationRects();
        rects.assign(_rects.begin(), _rects.end());
        if (_rects.empty()) {
            return false;
        }
        ean_decoder decoder("");

        vector<std::string> _decoded_info = decoder.rect_to_ucharlist(inarr, _rects);
        decoded_info.assign(_decoded_info.begin(), _decoded_info.end());

        return true;
    }

}
