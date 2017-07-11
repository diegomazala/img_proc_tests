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
	int64 start_program_time = cv::getTickCount();

	//check for the input parameter correctness
	if (argc < 3) 
	{
		std::cerr << "Usage: app.exe <bg_folder> <fg_folder> <output_folder>" << std::endl;
		std::cerr << "Abort." << std::endl;
		return EXIT_FAILURE;
	}

	cv::String bg_path(argv[1]); 
	cv::String fg_path(argv[2]);
	cv::String out_path(argv[3]);

	std::vector<cv::String> bg_files, fg_files;
	cv::glob(bg_path, bg_files, true); // recurse
	cv::glob(fg_path, fg_files, true); // recurse

	const uint16_t image_count = std::min(bg_files.size(), fg_files.size());

	for (int i = 0; i < image_count; ++i)
	{
		std::cout << "-------- " << i << "-------- " << std::endl;

		int64 start_time = cv::getTickCount();

		const std::string& filename_bg = bg_files[i];
		const std::string& filename_fg = fg_files[i];

		std::stringstream ss;
		ss << out_path << "/out_" << i << ".tif";
		const std::string filename_output = ss.str();

		std::cout << "Reading input images ..." << std::endl;

		//
		// read the image files in grayscale mode
		//
		cv::Mat frame_bg = cv::imread(filename_bg, CV_LOAD_IMAGE_GRAYSCALE);
		cv::Mat frame_fg = cv::imread(filename_fg, CV_LOAD_IMAGE_GRAYSCALE);
		cv::Mat frame_fg_rgb = cv::imread(filename_fg, CV_LOAD_IMAGE_UNCHANGED);
		//
		if (frame_bg.empty() || frame_fg.empty())
		{
			std::cerr << "Image could not be loaded. Abort" << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << "Subtracting background ..." << std::endl;

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

		std::cout << "Blurring image ..." << std::endl;

		//
		// smooth
		//
		cv::Mat smooth;
		cv::medianBlur(frame_abs_gray, smooth, 25);

		std::cout << "Threshold binary ..." << std::endl;

		//
		// binary threshold
		//
		cv::Mat binary_threshold;
		const uint8_t bits_per_sample = (smooth.depth() == CV_16U || smooth.depth() == CV_16S) ? 16 : 8;
		const uint32_t max_value = (uint32_t)pow(2, bits_per_sample);
		cv::threshold(smooth, binary_threshold, float(max_value) * 0.1f, max_value, cv::THRESH_BINARY);

		std::cout << "Floodfill ..." << std::endl;

		//
		// Floodfill from point (0, 0)
		//
		cv::Mat floodfill = binary_threshold.clone();
		cv::floodFill(floodfill, cv::Point(0, 0), cv::Scalar(255));
		//
		// Invert floodfilled image
		cv::Mat floodfill_inv;
		bitwise_not(floodfill, floodfill_inv);
		//
		// Combine the two images to get the foreground.
		cv::Mat floodfill_out = (binary_threshold | floodfill_inv);

		//
		// Apply mask to input fg image
		//
		cv::Mat segmentation;
		frame_fg_rgb.copyTo(segmentation, floodfill_out);

		cv::Mat alpha(frame_fg_rgb.rows, frame_fg_rgb.cols, CV_16UC1);
		floodfill_out.convertTo(alpha, CV_16UC1, 255);	// 8bits to 16bits

		cv::Mat src_imgs[] = { frame_fg_rgb, alpha };
		int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
		cv::Mat rgba = cv::Mat(frame_fg_rgb.rows, frame_fg_rgb.cols, CV_16UC4);
		cv::mixChannels(src_imgs, 2, &(rgba), 1, from_to, 4); 

		std::cout << "Saving result " << i << " ..." << std::endl;

		//
		// Save result
		//
		bool saved = cv::imwrite(filename_output, rgba);

		if (saved)
			std::cout << "Image " << i << " saved as " << filename_output << std::endl;
		else
			std::cout << "Error: Could not save image " << i << " : " << filename_output << std::endl;

		//
		// Time prints
		//
		int64 end_time = cv::getTickCount();
		double time_diff = double(end_time - start_time) / cv::getTickFrequency();
		std::cout << "Time spent in seconds :  " << time_diff << std::endl;
	}

	int64 end_program_time = cv::getTickCount();
	std::cout 
		<< "Total time in seconds :  " 
		<< double(end_program_time - start_program_time) / cv::getTickFrequency() << std::endl;

	return EXIT_SUCCESS;
}
