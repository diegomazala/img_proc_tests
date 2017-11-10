#include <iostream>
#include <vector>
#include "opencv2/highgui.hpp"


// work around conflicting definitions on 14.04 LTS with g++ 4.8.2
# define uint64 uint64_hack_
# define int64 int64_hack_

#include <tiff.h>
# undef uint64
# undef int64
# define uint64 uint64_hack_
# define int64 int64_hack_


static void equalization_per_channel(const cv::Mat& input_img, cv::Mat& output_img, double r, double g, double b)
{
	std::vector<cv::Mat> channels;
	cv::split(input_img, channels); //split the image into channels
	cv::normalize(channels[0], channels[0], 0, 65535.0 * b, cv::NORM_MINMAX, CV_16U);
	cv::normalize(channels[1], channels[1], 0, 65535.0 * g, cv::NORM_MINMAX, CV_16U);
	cv::normalize(channels[2], channels[2], 0, 65535.0 * r, cv::NORM_MINMAX, CV_16U);
	cv::merge(channels, output_img); //merge 3 channels 
}



int main(int argc, const char **argv)
{
	const std::string inFilename = argv[1];
	const std::string outFilename = argv[2];

	cv::Mat input = cv::imread(inFilename, cv::IMREAD_COLOR);	// 18 0 3
	if (input.empty())
	{
		std::cerr << "Cannot read image file: " << inFilename << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << ' ' 
		<< input.type() << ' '
		<< input.depth() << ' '
		<< input.channels() << std::endl;

	cv::Mat output = input;
	cv::imwrite(outFilename, output);

	return 0;

	int tags[] = { TIFFTAG_COMPRESSION, COMPRESSION_NONE };
	bool success = cv::imwrite("../data/_out_COMPRESSION_NONE.tif", input, std::vector<int>(tags, tags + 2));

	tags[1] = COMPRESSION_LZW;
	success = cv::imwrite("../data/_out_COMPRESSION_LZW.tif", input, std::vector<int>(tags, tags + 2));

	tags[1] = COMPRESSION_DEFLATE;
	success = cv::imwrite("../data/_out_COMPRESSION_DEFLATE.tif", input, std::vector<int>(tags, tags + 2));

	tags[1] = COMPRESSION_ADOBE_DEFLATE;
	success = cv::imwrite("../data/_out_COMPRESSION_ADOBE_DEFLATE.tif", input, std::vector<int>(tags, tags + 2));


	exit(0);
	cv::waitKey();

	const char* window_name = "Window";

	cv::namedWindow(window_name, CV_WINDOW_NORMAL);
	cv::resizeWindow(window_name, 720, 1280);
	cv::moveWindow(window_name, 0, 0);

	return EXIT_SUCCESS;
}





