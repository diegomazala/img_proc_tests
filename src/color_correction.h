
#ifndef _COLOR_CORRECTION_H_
#define _COLOR_CORRECTION_H_

#include "rigid_transformation.h"

#include "color_fitting.h"
#include "color_checker_picker.h"

typedef double Type;


static void show_window(const std::string& window_name, const cv::Mat& img, float window_scale)
{
	const float w = float(img.cols) * window_scale;
	const float h = float(img.rows) * window_scale;
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
	mat.resize(color_checker.meanColors.size(), 3);

	for (size_t i = 0; i < color_checker.meanColors.size(); i++)
	{
		mat(i, 0) = color_checker.meanColors[i][2];
		mat(i, 1) = color_checker.meanColors[i][1];
		mat(i, 2) = color_checker.meanColors[i][0];
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


#endif // _COLOR_CORRECTION_H_