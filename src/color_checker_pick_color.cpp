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
	std::cout << x << ' ' << y << std::endl;
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

	colorChecker.SetupWindow();
	cv::setMouseCallback(colorChecker.windowImage, onMouse, 0);
	colorChecker.MainLoop();
	colorChecker.ComputeMeanColors();

	std::stringstream ss;
	ss << filename.substr(0, filename.size() - 4) << ".rgbmat";
	colorChecker.Save(ss.str());

	return 0;
}