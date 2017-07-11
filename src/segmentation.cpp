//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/bgsegm.hpp"
#include <opencv2/highgui.hpp>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>

// Global variables



static void show_window(const std::string& window_name, const cv::Mat& img)
{
	const float w = float(img.cols / 5);
	const float h = w * (float(img.cols) / float(img.rows));
	cv::namedWindow(window_name, CV_WINDOW_NORMAL );
	cv::resizeWindow(window_name, (int)w, (int)h);
	//cv::moveWindow(window_name, 0, 0);
	cv::imshow(window_name, img);
}

int main(int argc, char* argv[])
{
	//check for the input parameter correctness
	if (argc < 3) 
	{
		std::cerr << "Usage: app.exe <bg_image> <fg_image> <output_filename>" << std::endl;
		std::cerr << "Abort." << std::endl;
		return EXIT_FAILURE;
	}

	const std::string filename_bg = argv[1];
	const std::string filename_fg = argv[2];
	const std::string filename_output = (argc > 2) ? argv[3] : "segmentation_result.tif";

	// read the image files in grayscale mode
	cv::Mat frame_bg = cv::imread(filename_bg, CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat frame_fg = cv::imread(filename_fg, CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat frame_fg_rgb = cv::imread(filename_fg, CV_LOAD_IMAGE_UNCHANGED);

	if (frame_bg.empty() || frame_fg.empty())
	{
		std::cerr << "Image could not be loaded. Abort" << std::endl;
		return EXIT_FAILURE;
	}


	//
	// image subtraction
	//
	cv::Mat frame_abs_diff;
	cv::absdiff(frame_bg, frame_fg, frame_abs_diff);

	cv::Mat frame_abs_gray;
	if (frame_abs_diff.depth() != 0)
		cv::cvtColor(frame_abs_diff, frame_abs_gray, CV_BGR2GRAY);
	else
		frame_abs_gray = frame_abs_diff;

	//
	// smooth
	//
	cv::Mat smooth;
	cv::medianBlur(frame_abs_gray, smooth, 25);

	//
	// binary threshold
	//
	cv::Mat binary_threshold;
	const uint8_t bits_per_sample = (smooth.depth() == CV_16U || smooth.depth() == CV_16S) ? 16 : 8;
	const uint32_t max_value = (uint32_t)pow(2, bits_per_sample);
	cv::threshold(smooth, binary_threshold, float(max_value) * 0.1f, max_value, cv::THRESH_BINARY);


	//show_window("bg", frame_bg);
	//show_window("fg", frame_fg);
	//show_window("abs_diff", frame_abs_diff);
	//show_window("binary_threshold", binary_threshold);
	//show_window("smooth", smooth);


	// Floodfill from point (0, 0)
	cv::Mat im_floodfill = binary_threshold.clone();
	floodFill(im_floodfill, cv::Point(0, 0), cv::Scalar(255));

	// Invert floodfilled image
	cv::Mat im_floodfill_inv;
	bitwise_not(im_floodfill, im_floodfill_inv);

	// Combine the two images to get the foreground.
	cv::Mat im_out = (binary_threshold | im_floodfill_inv);

	//show_window("im_floodfill", im_floodfill);
	//show_window("im_floodfill_inv", im_floodfill_inv);
	//show_window("im_out", im_out);

	cv::Mat segmentation;
	frame_fg_rgb.copyTo(segmentation, im_out);

	show_window("segmentation", segmentation);

	cv::waitKey(0);

	cv::imwrite(filename_output, segmentation);
	//cv::imwrite("../data/bg/bg_sub_abs.tif", frame_abs_diff);

	//destroy GUI windows
	cv::destroyAllWindows();

	return EXIT_SUCCESS;
}
