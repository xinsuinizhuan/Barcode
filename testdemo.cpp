#include "opencv2/highgui/highgui.hpp"
#include "testenv.h"
#include <string>
using namespace cv;

int main(int argc, char** argv) {
	//showExampleImg()
	//将识别的二维码保存
	String argument = "save_detections";
	printf("%d",argc);
	scanQRCode(argc, argv);
	return 0;
}