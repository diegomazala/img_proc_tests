#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <opencv2/opencv.hpp>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/xphoto.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "color_correction.h"



ColorCheckerPicker colorChecker;


static void onMouse(int event, int x, int y, int a, void* a_ptr)
{
	colorChecker.OnMouseEvent(event, x, y, a, a_ptr);
}

// Global variables
static const cv::String keys =
"{help h usage ?  |      |                                }"
"{img image       |      | input image                    }"
"{dir directory   |      | input folder                   }"
"{rgb rgb_matrix  |      | input rgb matrix '.rgbmat'     }"
"{tgt rgb_target  |      | target rgb matrix '.rgbmat'    }"
"{eq equalization |      | use equalization               }"
;

//
// Examples:
// ./color_correction.exe -img=../../data/figurante01/chart/Figurante_CAM08_7834.CR2.tiff -eq
// ./color_correction.exe -img=../../data/figurante01/chart/Figurante_CAM08_7834.CR2.tiff -rgb=../../data/figurante01/chart/Figurante_CAM08_7834.CR2.rgbmat -eq
// ./color_correction.exe -dir=../../data/casaco/fg/tiff/color_checker -eq
//
int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv, keys);
	parser.about("Color correction");
	if (parser.has("help"))
	{
		parser.printMessage();
		return EXIT_FAILURE;
	}

	bool do_equalization = parser.has("equalization");
	cv::String input_img_file = parser.get<cv::String>("img");
	cv::String input_folder = parser.get<cv::String>("dir");
	cv::String rgb_mat_src_file = parser.get<cv::String>("rgb");
	cv::String rgb_mat_dst_file = parser.get<cv::String>("tgt");
	
	std::vector<cv::String> input_files;
	if (!input_folder.empty())
		cv::glob(input_folder, input_files, true);
	
	if (input_files.empty())
		input_files.push_back(input_img_file);

	std::stringstream ss;
	
	// 
	// Rgb Fitting init
	//
	RgbFitting<Type> rgb_fitting;

	//
	// Runnig Color Ckecker Picker
	//
	if (rgb_mat_src_file.empty())
	{
		for each (const cv::String& in_file in input_files)
		{
			if (colorChecker.LoadImage(in_file))
			{
				input_img_file = in_file;
				break;
			}
		}
		
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

		auto s = colorChecker.meanColors[21];
		std::cout << "--Mean: " << s << std::endl;

		transform_image_per_channel(colorChecker.image, colorChecker.image, 122.0 / s[0], 122.0 / s[1], 122.0 / s[2]);

		colorChecker.ComputeMeanColors();

		std::cout << "--Mean: " << colorChecker.meanColors[21] << std::endl;
		//return EXIT_FAILURE;

		ss.str(std::string());
		ss.clear();
		ss << input_img_file.substr(0, input_img_file.find_last_of('.')) << ".rgbmat";
		colorChecker.Save(ss.str());

		matrix_from_color_checker_picker(colorChecker, rgb_fitting.source);
	}
	else
	{
		if (!matrix_from_file(rgb_mat_src_file, rgb_fitting.source))
			return EXIT_FAILURE;
	}


	if (!rgb_mat_dst_file.empty())
	{
		if (!matrix_from_file(rgb_mat_dst_file, rgb_fitting.target))
			return EXIT_FAILURE;
	}
	else
	{
		rgb_fitting.target = color_checker_matrix();
	}
	//
	// Matching size of the matrices
	//
	if (rgb_fitting.source.rows() != rgb_fitting.target.rows())
	{
		size_t min_size_rows = std::min(rgb_fitting.source.rows(), rgb_fitting.target.rows());
		size_t min_size_cols = std::min(rgb_fitting.source.cols(), rgb_fitting.target.cols());

		const auto source = rgb_fitting.source;
		const auto target = rgb_fitting.target;

		rgb_fitting.source.resize(min_size_rows, min_size_cols);
		rgb_fitting.target.resize(min_size_rows, min_size_cols);

		rgb_fitting.source = source.block(0, 0, min_size_rows, min_size_cols);
		rgb_fitting.target = target.block(0, 0, min_size_rows, min_size_cols);
	}

	if (rgb_fitting.source.rows() < 22)
	{
		std::cerr << "Error: You must have at least 22 colors set. Abort" << std::endl;
		return EXIT_FAILURE;
	}

	rgb_fitting.compute();

	for each (const cv::String& in_file in input_files)
	{
		cv::Mat input_img = cv::imread(in_file, cv::IMREAD_COLOR);

		if (input_img.empty())
			continue;

		cv::Mat output_img = cv::Mat(input_img.size(), input_img.type());

		std::cout << "Applying color correction..." << std::endl;
		transform_image_per_channel(output_img, input_img, rgb_fitting.functorResult);


		ss.str(std::string());
		ss.clear();
		ss << in_file.substr(0, in_file.find_last_of('.')) << "_corrected.tif";
		cv::String output_img_file = ss.str();

		std::cout << "Saving image corrected ... " << output_img_file << std::endl;
		if (do_equalization)
		{
			// using a specific gray block in color checker to shift rgb 
			double shift_red = rgb_fitting.target(21, 0) / rgb_fitting.source(21, 0);
			double shift_green = rgb_fitting.target(21, 1) / rgb_fitting.source(21, 1);
			double shift_blue = rgb_fitting.target(21, 2) / rgb_fitting.source(21, 2);

			std::vector<cv::Mat> channels;
			cv::split(input_img, channels);

			cv::Mat histog_img = cv::Mat(input_img.size(), input_img.type());
			equalization_per_channel(output_img, histog_img, shift_red, shift_green, shift_blue);

			show_window("Color_Correcion", histog_img, 0.2f);
			cv::imwrite(output_img_file, histog_img);
			
			int delay = (input_files.size() > 1) ? 1000 : -1;
			cv::waitKey(delay);
		}
		else
		{
			show_window("Color_Correcion", output_img, 0.2f);
			cv::imwrite(output_img_file, output_img);
			
			//cv::Mat cv_image_gamma(cv::Size(output_img.cols, output_img.rows), CV_16UC3);
			//output_img.convertTo(cv_image_gamma, -1, 2.222, 45);
			//cv::imwrite("../data/cv_image_gamma.tif", cv_image_gamma);

			int delay = (input_files.size() > 1) ? 1000 : -1;
			cv::waitKey(delay);
		}
	}
	
	cv::destroyAllWindows();
	return EXIT_SUCCESS;
}
