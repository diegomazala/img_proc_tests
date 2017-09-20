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
#include "color_fitting.h"
#include "color_checker_picker.h"
ColorCheckerPicker colorChecker;

typedef double Type;


static void show_window(const std::string& window_name, const cv::Mat& img)
{
	const float w = float(img.cols / 2);
	const float h = w * (float(img.cols) / float(img.rows));
	cv::namedWindow(window_name, CV_WINDOW_NORMAL);
	cv::resizeWindow(window_name, (int)w, (int)h);
	//cv::moveWindow(window_name, 0, 0);
	cv::imshow(window_name, img);
}

static void equalization_per_channel(const cv::Mat& input_img, cv::Mat& output_img, double r, double g, double b)
{
	std::vector<cv::Mat> channels;
	cv::split(input_img, channels); //split the image into channels
	cv::normalize(channels[0], channels[0], 0, 65535.0 * b, cv::NORM_MINMAX, CV_16U);
	cv::normalize(channels[1], channels[1], 0, 65535.0 * g, cv::NORM_MINMAX, CV_16U);
	cv::normalize(channels[2], channels[2], 0, 65535.0 * r, cv::NORM_MINMAX, CV_16U);
	cv::merge(channels, output_img); //merge 3 channels including the modified 1st channel into one image
}




static Eigen::Matrix<Type, 24, 3> color_checker_matrix()
{
	Eigen::Matrix<Type, 24, 3> rgb_matrix;
	rgb_matrix <<
	115, 82, 68,
	194, 150, 130,
	98, 122, 157,
	87, 108, 67,
	133, 128, 177,
	103, 189, 170,

	214, 126, 44,
	80, 91, 166,
	193, 90, 99,
	94, 60, 108,
	157, 188, 64,
	224, 163, 46,

	56, 61, 150,
	70, 148, 73,
	175, 54, 60,
	231, 199, 31,
	187, 86, 149,
	8, 133, 161,

	243, 243, 242,
	200, 200, 200,
	160, 160, 160,
	122, 122, 121,
	85, 85, 85,
	52, 52, 52;
	return rgb_matrix;
}

bool matrix_from_file(const std::string& filename, Eigen::Matrix<Type, Eigen::Dynamic, 3>& mat)
{
	std::ifstream infile(filename);
	if (infile.is_open())
	{
		int rows, cols;
		infile >> rows >> cols;

		mat.resize(rows, cols);

		if (rows != mat.rows() || cols != mat.cols())
		{
			std::cerr << "Error: Could not read rows and cols from matrix file. Abort." << std::endl;
			return false;
		}

		for (int i = 0; i < mat.rows(); ++i)
		{
			infile >> mat(i, 0) >> mat(i, 1) >> mat(i, 2);
		}
	}

	return true;
}


bool matrix_from_color_checker_picker(const ColorCheckerPicker& color_checker, Eigen::Matrix<Type, Eigen::Dynamic, 3>& mat)
{
	mat.resize(colorChecker.meanColors.size(), 3);

	for (size_t i = 0; i < colorChecker.meanColors.size(); i++)
	{
		mat(i, 0) = colorChecker.meanColors[i][2];
		mat(i, 1) = colorChecker.meanColors[i][1];
		mat(i, 2) = colorChecker.meanColors[i][0];
	}

	return true;
}


static void transform_image_per_channel(
	cv::Mat& output_img, 
	const cv::Mat& input_img, 
	const Eigen::Matrix<Type, 3, 2>& alpha_beta)
{
	std::vector<cv::Mat> channels;
	cv::split(input_img, channels); //split the image into channels

	for (int i = 0; i < 3; ++i)
	{
		channels[i].convertTo(channels[i], input_img.depth(), alpha_beta(2 - i, 0), alpha_beta(2 - i, 1));
	}

	cv::merge(channels, output_img); //merge 3 channels including the modified 1st channel into one image
}


bool test_color_fitting_from_file(int argc, char** argv)
{
	RgbFitting<Type> rgb_fitting;

	cv::Mat input_img = cv::imread(argv[1], CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat output_img = cv::Mat(input_img.size(), input_img.type());
	

	rgb_fitting.source = Eigen::Matrix<Type, 24, 3>();
	matrix_from_file(argv[2], rgb_fitting.source);

	if (argc > 3)
	{
		rgb_fitting.target = Eigen::Matrix<Type, 24, 3>();
		matrix_from_file(argv[3], rgb_fitting.target);
	}
	else
	{
		rgb_fitting.target = color_checker_matrix();
	}

	rgb_fitting.compute();

	std::cout << "Applying color correction..." << std::endl;

	transform_image_per_channel(output_img, input_img, rgb_fitting.functorResult);

	if (argc > 4)
	{
		std::stringstream ss;

		std::cout << "Saving image corrected ..." << std::endl;
		imwrite(argv[4], output_img);

		double shift_red = rgb_fitting.target(21, 0) / rgb_fitting.source(21, 0);
		double shift_green = rgb_fitting.target(21, 1) / rgb_fitting.source(21, 1);
		double shift_blue = rgb_fitting.target(21, 2) / rgb_fitting.source(21, 2);

		cv::Mat histog_img = cv::Mat(input_img.size(), input_img.type());
		equalization_per_channel(output_img, histog_img, shift_red, shift_green, shift_blue);
		ss.str(std::string());
		ss.clear();
		ss << argv[4] << "_equalized.tif";
		imwrite(ss.str(), histog_img);
	}
	return true;
}


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
		std::cerr << "Erro: You must set at least 22 colors. Abort" << std::endl;
		return EXIT_FAILURE;
	}


	rgb_fitting.compute();


	for each (const cv::String& in_file in input_files)
	{
		cv::Mat input_img = cv::imread(in_file, CV_LOAD_IMAGE_UNCHANGED);

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

			cv::Mat histog_img = cv::Mat(input_img.size(), input_img.type());
			equalization_per_channel(output_img, histog_img, shift_red, shift_green, shift_blue);
			imwrite(output_img_file, histog_img);
		}
		else
		{
			imwrite(output_img_file, output_img);
		}
	}
	


	

	


	return EXIT_SUCCESS;
}
