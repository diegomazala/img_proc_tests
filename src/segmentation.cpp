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
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

// Global variables
static const cv::String keys =
"{help h usage ?    |      | print this message		 }"
"{bg background     |      | background image or dir   }"
"{fg foreground     |      | foreground image or dir   }"
"{out output        |      | output image or dir		 }"
"{sub subtraction   | 0.1  | subtraction percentage    }"
"{m morphological   | 21   | morphological element size    }"
"{i intermediate    |      | save intermediate files    }"
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

	std::vector<cv::String> bg_files, fg_files;

	fs::is_directory(bg.c_str()) ? cv::glob(bg, bg_files, true) : bg_files.push_back(bg);
	fs::is_directory(fg.c_str()) ? cv::glob(fg, fg_files, true) : fg_files.push_back(fg);

	std::string output_folder, 
				output_filename, 
				output_extension = ".tif";
	if (fs::is_directory(out.c_str()))
	{
		output_folder = out;
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

	float subtraction_percentage = parser.get<float>("sub");;
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
		ss << output_folder << output_filename << '_' << i << output_extension;
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
		if (save_intermediate)
		{
			ss.str(std::string());
			ss.clear();
			ss << filename_output << "_s0_frame_abs_gray" << output_extension;
			std::cout << "Saving: " << ss.str() << std::endl;
			cv::imwrite(ss.str(), frame_abs_gray);
		}
				
		std::cout << "Threshold binary ..." << std::endl;
		//
		// binary threshold
		//
		cv::Mat binary_threshold;
		const uint8_t bits_per_sample = (frame_abs_gray.depth() == CV_16U || frame_abs_gray.depth() == CV_16S) ? 16 : 8;
		const uint32_t max_value = (uint32_t)pow(2, bits_per_sample);
		cv::threshold(frame_abs_gray, binary_threshold, float(max_value) * 0.1f, max_value, cv::THRESH_BINARY);
		if (save_intermediate)
		{
			ss.str(std::string());
			ss.clear();
			ss << filename_output << "_s1_threshold" << output_extension;
			std::cout << "Saving: " << ss.str() << std::endl;
			cv::imwrite(ss.str(), binary_threshold);
		}

		//
		// morphological filters
		//
		cv::Mat erode_img, dilate_img;
		cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS,
			cv::Size(2 * morphological_kernel_size + 1, 2 * morphological_kernel_size + 1),
			cv::Point(morphological_kernel_size, morphological_kernel_size));
		cv::dilate(binary_threshold, dilate_img, element);
		cv::erode(dilate_img, erode_img, element);
		if (save_intermediate)
		{
			ss.str(std::string());
			ss.clear();
			ss << filename_output << "_s2_morph_filters" << output_extension;
			std::cout << "Saving: " << ss.str() << std::endl;
			cv::imwrite(ss.str(), erode_img);
		}


		std::cout << "Floodfill ..." << std::endl;
		//
		// Floodfill from point (0, 0)
		//
		cv::Mat floodfill = erode_img.clone();
		cv::floodFill(floodfill, cv::Point(0, 0), cv::Scalar(255));
		//
		// Invert floodfilled image
		cv::Mat floodfill_inv;
		bitwise_not(floodfill, floodfill_inv);
		if (save_intermediate)
		{
			ss.str(std::string());
			ss.clear();
			ss << filename_output << "_s3_floodfill_inv" << output_extension;
			std::cout << "Saving: " << ss.str() << std::endl;
			cv::imwrite(ss.str(), floodfill_inv);
		}
		//
		// Combine the two images to get the foreground.
		cv::Mat floodfill_out = (erode_img | floodfill_inv);
		if (save_intermediate)
		{
			ss.str(std::string());
			ss.clear();
			ss << filename_output << "_s4_combine_floodfill" << output_extension;
			std::cout << "Saving: " << ss.str() << std::endl;
			cv::imwrite(ss.str(), floodfill_out);
		}
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
