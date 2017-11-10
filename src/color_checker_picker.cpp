#include "color_checker_picker.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <fstream>

static cv::Point center_of_rect(const cv::Rect& r)
{
	return cv::Point(r.width / 2 + r.x, r.height / 2 + r.y);
}

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


cv::Rect ColorCheckerPicker::FloodFill(int x, int y)
{
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
	
	int area = floodFill(
				image, seed, newVal, &ccomp, 
				cv::Scalar(lo, lo, lo),
				cv::Scalar(up, up, up), flags);

	//cv::rectangle(dst, ccomp, cv::Scalar(255, 255, 255), 10, cv::LINE_8);
	//cv::circle(dst, seed, 10, cv::Scalar(255, 255, 255), 10, cv::LINE_8);

	return ccomp;
}

void ColorCheckerPicker::OnMouseEvent(int event, int x, int y, int, void*)
{
	if (event != cv::EVENT_LBUTTONDOWN)
		return;

	cv::Mat dst_img = isColor ? image : gray;

	cv::Rect ccomp = FloodFill(x, y);
	cv::circle(dst_img, center_of_rect(ccomp), 10, cv::Scalar(255, 255, 255), 10, cv::LINE_8);

	colorRects.push_back(ccomp);

	int min_x = colorRects[0].x;
	int max_x = colorRects[0].x;
	int min_y = colorRects[0].y;
	int max_y = colorRects[0].y;

	for (int i = 1; i < colorRects.size(); ++i)
	{
		if (colorRects[i].x < min_x)
			min_x = colorRects[i].x;

		if (colorRects[i].x > max_x)
			max_x = colorRects[i].x;

		if (colorRects[i].y < min_y)
			min_y = colorRects[i].y;
		
		if (colorRects[i].y > max_y)
			max_y = colorRects[i].y;
	}

	int rect_width = colorRects[0].width;
	int rect_height = colorRects[0].height;

	max_x += rect_width;
	max_y += rect_height;

	cv::Rect big_rect(min_x, min_y, max_x - min_x, max_y - min_y);

	int width_step = big_rect.width / 6;
	int height_step = big_rect.height / 4;

	

	if (colorRects.size() > 2)
	{
		colorRects.clear();
		std::vector<cv::Point> seeds;

		//cv::rectangle(dst_img, big_rect, cv::Scalar(255, 255, 0), 10, cv::LINE_8);

		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 6; ++j)
				seeds.push_back(
					cv::Point(min_x + j * width_step + width_step / 2, min_y + i * height_step + height_step / 2));

		for (int i = 0; i < seeds.size() - 1; ++i)
		{
			const cv::Rect rect = FloodFill(seeds[i].x, seeds[i].y);
			colorRects.push_back(rect);
			cv::rectangle(dst_img, rect, cv::Scalar(255, 255, 255), 10, cv::LINE_8);
		}
	}


	cv::imshow(windowImage, dst_img);
}


void ColorCheckerPicker::OnKeyboard(char c)
{
	switch (c)
	{
	case 'c':
		ColorGraySwitch();
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


void ColorCheckerPicker::Reset()
{
	std::cout << "Pipeline Reset" << std::endl;
	image0.copyTo(image);
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
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
		isColor = false;
	}
	else
	{
		std::cout << "Color mode is set\n";
		image0.copyTo(image);
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

	cv::destroyWindow(windowImage);
}


void ColorCheckerPicker::ComputeMeanColors()
{
	int i = 0;
	meanColors.clear();
	for (const cv::Rect& rect : colorRects)
	{
		//std::cout << i++ << " : " << rect.x << ", " << rect.y << " - "  << rect.width << ", " << rect.height << std::endl;
		const cv::Scalar s = cv::mean(image(rect));
		meanColors.push_back(s);
		//std::cout << "MeanColor: "  << s << std::endl;
	}
}