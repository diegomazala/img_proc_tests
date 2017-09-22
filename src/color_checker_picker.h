#ifndef __COLOR_CHECKER_PICKER__
#define __COLOR_CHECKER_PICKER__

#include "opencv2/core.hpp"
#include <vector>




class ColorCheckerPicker
{
public: 
	ColorCheckerPicker();

	bool LoadImage(const std::string& fileimage);
	void Save(const std::string& rgbmat_filename);
	void OnMouseEvent(int event, int x, int y, int, void*);
	void OnKeyboard(char c);
	void UseMaskSwitch();
	void Reset();
	void SetFillMode(int fill_mode); // 0, 1, 2
	void SetConnectivity(int conn); // 4 or 8
	void ColorGraySwitch();
	void SetupWindow();
	void MainLoop();
	void ComputeMeanColors();

	cv::Mat image, image0, gray, mask;
	int ffillMode = 1;
	int loDiff = 20, upDiff = 10;
	int connectivity = 4;
	int isColor = true;
	bool useMask = false;
	int newMaskVal = 255;
	std::vector<cv::Rect> colorRects;
	const char* windowImage = "color_checker_picker_image";
	const char* windowMask = "color_checker_picker_mask";
	std::vector<cv::Scalar> meanColors;
};

#endif // __COLOR_CHECKER_PICKER__