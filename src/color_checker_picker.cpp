#include "color_checker_picker.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>



ColorCheckerPicker::ColorCheckerPicker()
{

}


bool ColorCheckerPicker::LoadImage(const std::string& fileimage)
{
	image0 = cv::imread(fileimage, cv::IMREAD_COLOR);

	if (!image0.empty())
	{
		image0.copyTo(image);
		cvtColor(image0, gray, cv::COLOR_BGR2GRAY);
		mask.create(image0.rows + 2, image0.cols + 2, CV_8UC1);
		return true;
	}
	return false;
}


void ColorCheckerPicker::Save(const std::string& rgbmat_filename)
{
	std::ofstream out_file(rgbmat_filename);
	if (out_file.is_open())
	{
		out_file << std::fixed << colorRects.size() << " 3" << std::endl;

		for (const cv::Rect& rect : colorRects)
		{
			auto mean = cv::mean(image(rect));
			out_file << mean[2] << ' ' << mean[1] << ' ' << mean[0] << std::endl;
		}
	}
	out_file.close();
}


void ColorCheckerPicker::OnMouseEvent(int event, int x, int y, int, void*)
{
	if (event != cv::EVENT_LBUTTONDOWN)
		return;

	std::cout << x << ' ' << y << std::endl;

	cv::Point seed(x, y);
	int lo = ffillMode == 0 ? 0 : loDiff;
	int up = ffillMode == 0 ? 0 : upDiff;
	int flags = connectivity + (newMaskVal << 8) + (ffillMode == 1 ? cv::FLOODFILL_FIXED_RANGE : 0);

	cv::Vec3b pixel = image.at<cv::Vec3b>(y, x);
	int b = pixel.val[0];	// (unsigned)theRNG() & 255;
	int g = pixel.val[1];	// (unsigned)theRNG() & 255;
	int r = pixel.val[2];	// (unsigned)theRNG() & 255;
	cv::Rect ccomp;
	cv::Scalar newVal = isColor ? cv::Scalar(b, g, r) : cv::Scalar(r*0.299 + g*0.587 + b*0.114);
	cv::Mat dst = isColor ? image : gray;
	int area;
	if (useMask)
	{
		cv::threshold(mask, mask, 1, 128, cv::THRESH_BINARY);
		area = floodFill(dst, mask, seed, newVal, &ccomp, cv::Scalar(lo, lo, lo),
			cv::Scalar(up, up, up), flags);

		cv::imshow(windowMask, mask);
	}
	else
	{
		area = floodFill(dst, seed, newVal, &ccomp, cv::Scalar(lo, lo, lo),
			cv::Scalar(up, up, up), flags);

		cv::rectangle(dst, ccomp, cv::Scalar(255, 255, 255), 10, cv::LINE_8);
	}

	colorRects.push_back(ccomp);

	cv::imshow(windowImage, dst);
	//std::cout << area << " pixels were repainted => " << ccomp.x << ',' << ccomp.y << ',' << ccomp.width << ',' << ccomp.height << std::endl;
}


void ColorCheckerPicker::OnKeyboard(char c)
{
	switch (c)
	{
	case 'c':
		ColorGraySwitch();
		break;
	case 'm':
		UseMaskSwitch();
		break;
	case 'r':
		Reset();
		break;
	case 's':
		std::cout << "Simple floodfill mode is set\n";
		SetFillMode(0);
		break;
	case 'f':
		std::cout << "Fixed Range floodfill mode is set\n";
		SetFillMode(1);
		break;
	case 'g':
		std::cout << "Gradient (floating range) floodfill mode is set\n";
		SetFillMode(2);
		break;
	case '4':
		std::cout << "4-connectivity mode is set\n";
		SetConnectivity(4);
		break;
	case '8':
		std::cout << "8-connectivity mode is set\n";
		SetConnectivity(8);
		break;
	}
}


void ColorCheckerPicker::UseMaskSwitch()
{
	if (useMask)
	{
		cv::destroyWindow(windowMask);
		useMask = false;
	}
	else
	{
		cv::namedWindow(windowMask, 0);
		mask = cv::Scalar::all(0);
		cv::imshow(windowMask, mask);
		useMask = true;
	}
}

void ColorCheckerPicker::Reset()
{
	std::cout << "Pipeline Reset" << std::endl;
	image0.copyTo(image);
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	mask = cv::Scalar::all(0);
	colorRects.clear();
}

void ColorCheckerPicker::SetFillMode(int fill_mode) // 0, 1, 2
{
	ffillMode = fill_mode;
}

void ColorCheckerPicker::SetConnectivity(int conn) // 4 or 8
{
	connectivity = conn;
}

void ColorCheckerPicker::ColorGraySwitch()
{
	if (isColor)
	{
		std::cout << "Grayscale mode is set\n";
		cv::cvtColor(image0, gray, cv::COLOR_BGR2GRAY);
		mask = cv::Scalar::all(0);
		isColor = false;
	}
	else
	{
		std::cout << "Color mode is set\n";
		image0.copyTo(image);
		mask = cv::Scalar::all(0);
		isColor = true;
	}
}




void ColorCheckerPicker::SetupWindow()
{
	//
	// Color Checker Picker UI
	//
	const float w = float(image0.cols / 5);
	const float h = float(image0.rows / 5);
	cv::namedWindow(windowImage, 0);
	cv::resizeWindow(windowImage, (int)w, (int)h);

	cv::createTrackbar("lo_diff", windowImage, &loDiff, 255, 0);
	cv::createTrackbar("up_diff", windowImage, &upDiff, 255, 0);
	//cv::setMouseCallback("image", onMouse, 0);
}

void ColorCheckerPicker::MainLoop()
{
	for (;;)
	{
		cv::imshow(windowImage, isColor ? image : gray);

		char c = (char)cv::waitKey(0);
		if (c == 27 || c == 13)
		{
			std::cout << "Exiting ...\n";
			break;
		}

		OnKeyboard(c);
	}
}


void ColorCheckerPicker::ComputeMeanColors()
{
	meanColors.clear();
	for (const cv::Rect& rect : colorRects)
	{
		meanColors.push_back(cv::mean(image(rect)));
	}
}