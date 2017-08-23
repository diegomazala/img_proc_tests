//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/bgsegm.hpp"
#include <opencv2/highgui.hpp>
//C
#include <stdio.h>

#include "background_subtraction.h"

//C++
#include <iostream>
#include <sstream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

// Global variables
static const cv::String keys =
"{help h usage ?    |      | print this message		 }"
"{bg background     |      | background image or dir   }"
"{fg foreground     |      | foreground image or dir   }"
"{out output        |      | output image or dir		 }"
"{m morphological   | 21   | morphological element size    }"
"{i intermediate    |      | save intermediate files    }"
"{s saturate        |      | saturate abs diff    }"
;


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
	//
	// command line parser
	//
	cv::CommandLineParser parser(argc, argv, keys);
	parser.about("Application name v1.0.0");

	cv::String bg = parser.get<cv::String>("bg");
	cv::String fg = parser.get<cv::String>("fg");
	cv::String out = parser.get<cv::String>("out");

	const bool out_folder = fs::is_directory(out.c_str());
	const bool save_intermediate = parser.has("intermediate");
	const bool saturate_abs_diff = parser.has("saturate");

	std::vector<cv::String> bg_files, fg_files;

	fs::is_directory(bg.c_str()) ? cv::glob(bg, bg_files, true) : bg_files.push_back(bg);
	fs::is_directory(fg.c_str()) ? cv::glob(fg, fg_files, true) : fg_files.push_back(fg);

	std::string output_folder, 
				output_filename, 
				output_extension = ".tif";
	if (fs::is_directory(out.c_str()))
	{
		output_folder = out;
		output_filename = "out";
	}
	else
	{
		fs::path p(out.c_str());
		output_folder = p.parent_path().string() + '/';
		fs::create_directory(output_folder);
		if (p.has_filename())
			output_filename = p.stem().string();
		if (p.has_extension())
			output_extension = p.extension().string();
		if (output_extension.size() < 3)	// some error ocurred
			output_extension = ".tif";
	}
	

	std::cout << std::endl
		<< "path      : " << output_folder << std::endl
		<< "filename  : " << output_filename << std::endl
		<< "extension : " << output_extension << std::endl
		<< std::endl;

	int morphological_kernel_size = parser.get<int>("m");;

	int64 start_program_time = cv::getTickCount();


	const uint16_t image_count = std::min(bg_files.size(), fg_files.size());

	for (int i = 0; i < image_count; ++i)
	{
		std::cout << "-------- " << i << "-------- " << std::endl;

		int64 start_time = cv::getTickCount();

		const std::string& filename_bg = bg_files[i];
		const std::string& filename_fg = fg_files[i];

		std::stringstream ss;
		ss << output_filename << '_' << i ;
		const std::string output_basename = ss.str();


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

		BackgroundSubtraction bg_sub;
		bg_sub.saveIntermediateFiles(save_intermediate);
		bg_sub.saturateAbs(saturate_abs_diff);
		bg_sub.setMorphologicalKernelSize(morphological_kernel_size);
		bg_sub.setResultFileName(output_folder, output_basename, output_extension);
		bg_sub.process(frame_bg, frame_fg);
		cv::Mat rgba;
		bg_sub.compose(frame_fg_rgb, rgba);

		
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
