#ifndef __COLOR_CHECKER_PICKER__
#define __COLOR_CHECKER_PICKER__

#include "opencv2/core.hpp"
#include <vector>




class ColorCheckerPicker
{
public: 
	ColorCheckerPicker();

	void LoadImage(const std::string& fileimage);
	void Save();
	void OnMouseEvent(int event, int x, int y, int, void*);
	void OnKeyboard(char c);
	void UseMaskSwitch();
	void Reset();
	void SetFillMode(int fill_mode); // 0, 1, 2
	void SetConnectivity(int conn); // 4 or 8
	void ColorGraySwitch();

	cv::Mat image, image0, gray, mask;
	int ffillMode = 1;
	int loDiff = 20, upDiff = 20;
	int connectivity = 4;
	int isColor = true;
	bool useMask = false;
	int newMaskVal = 255;
	std::vector<cv::Rect> colorRects;
};

#endif // __COLOR_CHECKER_PICKER__