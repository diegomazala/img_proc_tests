#include "color_checker_picker.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

ColorCheckerPicker colorChecker;

static void help()
{
	std::cout << "\nThis program demonstrated the floodFill() function\n"
		"Call:\n"
		"./ffilldemo [image_name -- Default: ../data/fruits.jpg]\n" << std::endl;
	std::cout << "Hot keys: \n"
		"\tESC - quit the program\n"
		"\tc - switch color/grayscale mode\n"
		"\tm - switch mask mode\n"
		"\tr - restore the original image\n"
		"\ts - use null-range floodfill\n"
		"\tf - use gradient floodfill with fixed(absolute) range\n"
		"\tg - use gradient floodfill with floating(relative) range\n"
		"\t4 - use 4-connectivity mode\n"
		"\t8 - use 8-connectivity mode\n" << std::endl;
}




static void show_window(const std::string& window_name, const cv::Mat& img)
{
	const float w = float(img.cols / 2);
	const float h = w * (float(img.cols) / float(img.rows));
	cv::namedWindow(window_name, CV_WINDOW_NORMAL);
	cv::resizeWindow(window_name, (int)w, (int)h);
	//cv::moveWindow(window_name, 0, 0);
	cv::imshow(window_name, img);
}



static void onMouse(int event, int x, int y, int a, void* a_ptr)
{
	if (event != cv::EVENT_LBUTTONDOWN)
		return;

	colorChecker.OnMouseEvent(event, x, y, a, a_ptr);
}



int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv,
		"{help h | | show help message}{@image|../data/fruits.jpg| input image}"
	);

	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	std::string filename = parser.get<std::string>("@image");
	colorChecker.LoadImage(filename);
	if (colorChecker.image0.empty())
	{
		std::cout << "Image empty\n";
		parser.printMessage();
		return 0;
	}

	const float w = float(colorChecker.image0.cols / 5);
	const float h = float(colorChecker.image0.rows / 5);
	cv::namedWindow("image", 0);
	cv::resizeWindow("image", (int)w, (int)h);

	cv::createTrackbar("lo_diff", "image", &colorChecker.loDiff, 255, 0);
	cv::createTrackbar("up_diff", "image", &colorChecker.upDiff, 255, 0);
	cv::setMouseCallback("image", onMouse, 0);

	for (;;)
	{
		cv::imshow("image", colorChecker.isColor ? colorChecker.image : colorChecker.gray);

		char c = (char)cv::waitKey(0);
		if (c == 27)
		{
			std::cout << "Exiting ...\n";
			break;
		}
		
		colorChecker.OnKeyboard(c);
	}

	colorChecker.Save();

	return 0;
}