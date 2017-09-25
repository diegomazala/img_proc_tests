

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>


// Global variables
static const cv::String keys =
"{help h usage ?    |      | Usage: app.exe [image] [radius] }"
"{@image            |      | image input file   }"
"{r radius          | 41   | radius    }"
"{c count           | 3    | max spots count    }"
;




static void show_window(const std::string& window_name, const cv::Mat& img, float window_scale = 1.0f)
{
	const float w = float(img.cols) * window_scale;
	const float h = float(img.rows) * window_scale;
	cv::namedWindow(window_name, CV_WINDOW_NORMAL);
	cv::resizeWindow(window_name, (int)w, (int)h);
	cv::moveWindow(window_name, 0, 0);
	cv::imshow(window_name, img);
}


int main(int argc, char** argv)
{
	//
	// command line parser
	//
	cv::CommandLineParser parser(argc, argv, keys);
	parser.about("Application name v1.0.0");
	std::string filename = parser.get<std::string>("@image");
	int radius = parser.get<int>("radius");
	int max_spots_count = parser.get<int>("count");
	
	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}


	cv::Mat image = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
	if (image.empty())
	{
		std::cerr << "Error: Could not load image file. Abort" << std::endl;
		return EXIT_FAILURE;
	}
	
	cv::Mat blurred(image.size(), image.type());
	cv::GaussianBlur(image, blurred, cv::Size(radius, radius), 0);

	cv::Mat mask(image.size(), image.type(), cv::Scalar(255));

	double minVal;
	double maxVal;
	cv::Point minIdx;
	cv::Point maxIdx;

	for (int i = 0; i < max_spots_count; ++i)
	{
		cv::minMaxLoc(blurred, &minVal, &maxVal, &minIdx, &maxIdx, mask);

		cv::circle(image, maxIdx, radius, cv::Scalar(255), 10);

		show_window("Brightest Spot", image, 0.15f);

		cv::circle(mask, maxIdx, radius * 2, cv::Scalar(0), cv::FILLED);
	}


	cv::waitKey();
	return 0;
}
