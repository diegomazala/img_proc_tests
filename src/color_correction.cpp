#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "color_correction.h"
#include "color_fitting.h"

static void show_window(const std::string& window_name, const cv::Mat& img)
{
	const float w = float(img.cols / 2);
	const float h = w * (float(img.cols) / float(img.rows));
	cv::namedWindow(window_name, CV_WINDOW_NORMAL);
	cv::resizeWindow(window_name, (int)w, (int)h);
	//cv::moveWindow(window_name, 0, 0);
	cv::imshow(window_name, img);
}

typedef double Type;

void test_with_eigen_per_channel()
{
	Eigen::Matrix<Type, 24, 1> A, b;
	Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> x;
	

	A << 
	80,
	164,
	73,
	70,
	99,
	108,
	177,
	43,
	152,
	57,
	149,
	191,
	0, 
	79,
	130,
	201,
	142,
	3, 
	209,
	178,
	140,
	95,
	59,
	30;

	b << 
	115,
	194,
	98, 
	87, 
	133,
	103,
	214,
	80, 
	193,
	94, 
	157,
	224,
	56, 
	70, 
	175,
	231,
	187,
	8,
	243,
	200,
	160,
	122,
	85, 
	52; 

	x = A.colPivHouseholderQr().solve(b);

	std::cout
		<< std::fixed << std::endl
		<< "Transform Matrix : " << std::endl
		<< x << std::endl
		<< std::endl;

	std::cout << "Rows/Cols: " << x.rows() << ' ' << x.cols() << std::endl;

	std::cout
		<< std::endl
		<< A * x
		<< std::endl << std::endl;

}



void test_with_eigen_per_channels()
{
	Eigen::Matrix<Type, 24, 3> src_rgb, dst_rgb;
	
	Eigen::Matrix<Type, 24, 1> A, b;
	Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> x;

	src_rgb <<
		80, 60, 68,
		164, 135, 155,
		73, 102, 179, 
		70, 84, 68,
		99, 107, 200, 
		108, 173, 198,
		177, 108, 62, 
		43, 70, 194,
		152, 66, 109, 
		57, 43, 116,
		149, 177, 89, 
		191, 145, 61, 
		0, 45, 171,
		79, 132, 87,
		130, 38, 60,
		201, 185, 61, 
		142, 62, 169, 
		3, 109, 181,
		209, 215, 240,
		178, 186, 222,
		140, 150, 191,
		95, 102, 137, 
		59, 63, 88,
		30, 32, 45;

	dst_rgb <<
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

	for (int i = 0; i < 3; ++i)
	{
		std::cout << "-------------- " << i << std::endl;
		A = src_rgb.col(i);
		b = dst_rgb.col(i);

		//x = A.colPivHouseholderQr().solve(b);
		x = A.fullPivLu().solve(b);
		double relative_error = (A*x - b).norm() / b.norm(); // norm() is L2 norm
		std::cout << "The relative error is:\n" << relative_error << std::endl;

		std::cout
			<< std::fixed << std::endl
			<< "Transform Matrix : " << std::endl
			<< x << std::endl
			<< std::endl;
		std::cout << "Rows/Cols: " << x.rows() << ' ' << x.cols() << std::endl;
		std::cout
			<< std::endl
			<< A * x
			<< std::endl << std::endl;
	}

}


Eigen::Matrix<Type, 4, 4> test_with_eigen()
{
	Eigen::Matrix<Type, 24, 3> a, b;
	Eigen::Matrix<Type, 3, 24> x;

	std::vector<Eigen::Matrix<Type, 3, 1>> src_rgb, dst_rgb;

	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(80, 60, 68));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(164, 135, 155));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(73, 102, 179));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(70, 84, 68));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(99, 107, 200));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(108, 173, 198));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(177, 108, 62));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(43, 70, 194));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(152, 66, 109));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(57, 43, 116));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(149, 177, 89));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(191, 145, 61));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(0, 45, 171));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(79, 132, 87));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(130, 38, 60));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(201, 185, 61));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(142, 62, 169));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(3, 109, 181));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(209, 215, 240));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(178, 186, 222));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(140, 150, 191));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(95, 102, 137));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(59, 63, 88));
	src_rgb.push_back(Eigen::Matrix<Type, 3, 1>(30, 32, 45));


	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(115, 82, 68));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(194, 150, 130));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(98, 122, 157));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(87, 108, 67));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(133, 128, 177));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(103, 189, 170));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(214, 126, 44));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(80, 91, 166));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(193, 90, 99));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(94, 60, 108));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(157, 188, 64));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(224, 163, 46));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(56, 61, 150));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(70, 148, 73));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(175, 54, 60));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(231, 199, 31));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(187, 86, 149));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(8, 133, 161));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(243, 243, 242));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(200, 200, 200));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(160, 160, 160));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(122, 122, 121));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(85, 85, 85));
	dst_rgb.push_back(Eigen::Matrix<Type, 3, 1>(52, 52, 52));

	//
	// Computing rigid transformation
	// 
	Eigen::Matrix<Type, 4, 4> transform;
	compute_rigid_transformation(src_rgb, dst_rgb, transform);

	std::cout
		<< std::fixed << std::endl
		<< "Transform Matrix : " << std::endl
		<< transform << std::endl
		<< std::endl;

	//for (std::size_t i = 0; i < src_rgb.size(); ++i)
	//{
	//	const Eigen::Matrix<Type, 4, 1>& v = src_rgb[i].homogeneous();

	//	Eigen::Matrix<Type, 4, 1> tv = transform.matrix() * v;
	//	tv /= tv.w();

	//	// (tv.head<3>());

	//	std::cout << (tv.head<3>()).transpose() << std::endl;
	//}

	return transform;
}


void test_with_opencv()
{
	float m3x3[3][4] = {
		{ 0.998631f, -0.051762f,  0.007566f, 31.907135, },
		{ 0.051362f,  0.997627f,  0.045840f, 7.402893, },
		{ -0.009920f, -0.045389f,  0.998920f, -9.333534 } };
	cv::Mat R(3, 4, CV_32FC1, m3x3);

	//float m3x3[3][3] = {
	//	{ 0.998631f, -0.051762f,  0.007566f, },
	//	{ 0.051362f,  0.997627f,  0.045840f, },
	//	{ -0.009920f, -0.045389f,  0.998920f } };
	//cv::Mat R(3, 3, CV_32FC1, m3x3);

	float src_rgb[24][3] = {
	{80, 60, 68,},
	{164, 135, 155,},
	{73, 102, 179,},
	{70, 84, 68,},
	{99, 107, 200,},
	{108, 173, 198,},
	{177, 108, 62,},
	{43, 70, 194,},
	{152, 66, 109,},
	{57, 43, 116,},
	{149, 177, 89,},
	{191, 145, 61,},
	{0, 45, 171,},
	{79, 132, 87,},
	{130, 38, 60,},
	{201, 185, 61,},
	{142, 62, 169,},
	{3, 109, 181,},
	{209, 215, 240,},
	{178, 186, 222,},
	{140, 150, 191,},
	{95, 102, 137,},
	{59, 63, 88,},
	{30, 32, 45,} };

	float dst_rgb[24][3] = {
	{115, 82, 68,},
	{194, 150, 130,},
	{98, 122, 157,},
	{87, 108, 67,},
	{133, 128, 177,},
	{103, 189, 170,},
	{214, 126, 44,},
	{80, 91, 166,},
	{193, 90, 99,},
	{94, 60, 108,},
	{157, 188, 64,},
	{224, 163, 46,},
	{56, 61, 150,},
	{70, 148, 73,},
	{175, 54, 60,},
	{231, 199, 31,},
	{187, 86, 149,},
	{8, 133, 161,},
	{243, 243, 242,},
	{200, 200, 200,},
	{160, 160, 160,},
	{122, 122, 121,},
	{85, 85, 85,},
	{52, 52, 52} };

	cv::Mat src_img(6, 4, CV_32FC3, src_rgb);
	cv::Mat dst_img(6, 4, CV_32FC3, dst_rgb);
	cv::Mat dst_img_computed = cv::Mat::zeros(6, 4, CV_32FC3);

	cv::transform(src_img, dst_img_computed, R);

	//std::cout << dst_img_computed << std::endl;

	for (int i = 0; i < dst_img_computed.rows * dst_img_computed.cols; ++i)
	{
		const cv::Vec3f pixel = dst_img_computed.at<cv::Vec3f>(i);
		std::cout << pixel[0] << ' ' << pixel[1] << ' ' << pixel[2] << std::endl;
	}

}

void test_color_fitting()
{	
	RgbFitting<Type> rgb_fitting;

	rgb_fitting.source = Eigen::Matrix<Type, 24, 3>();
	rgb_fitting.source <<
		80, 60, 68,
		164, 135, 155,
		73, 102, 179,
		70, 84, 68,
		99, 107, 200,
		108, 173, 198,
		177, 108, 62,
		43, 70, 194,
		152, 66, 109,
		57, 43, 116,
		149, 177, 89,
		191, 145, 61,
		0, 45, 171,
		79, 132, 87,
		130, 38, 60,
		201, 185, 61,
		142, 62, 169,
		3, 109, 181,
		209, 215, 240,
		178, 186, 222,
		140, 150, 191,
		95, 102, 137,
		59, 63, 88,
		30, 32, 45;

	rgb_fitting.target = Eigen::Matrix<Type, 24, 3>();
	rgb_fitting.target <<
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

	
	rgb_fitting.compute();
}

// @function main 
int main(int argc, char** argv)
{

	test_color_fitting();
	return EXIT_SUCCESS;

	//test_with_eigen_per_channels();
	//return EXIT_SUCCESS;

	//test_with_opencv();
	//return EXIT_SUCCESS;

	//Eigen::Matrix<Type, 4, 4> transform = test_with_eigen();
	//return EXIT_SUCCESS;

	cv::Mat input_img = cv::imread(argv[1], CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat output_img = cv::Mat(input_img.size(), input_img.type());





	float m3x3[3][4] = {
		{0.998631f, -0.051762f,  0.007566f, 31.907135, },
		{0.051362f,  0.997627f,  0.045840f, 7.402893, },
		{-0.009920f, -0.045389f,  0.998920f, -9.333534}};

	//31.907135  7.402893 - 9.333534

	cv::Mat R(3, 4, CV_32FC1, m3x3);

	//float m3[3] = { 80, 60, 68};
	//cv::Mat src(1, 3, CV_32FC1, m3);
	//cv::Mat dst = cv::Mat::zeros(3, 1, CV_32FC1);

	//const cv::Vec3b pixel_src(80, 60, 68);
	//const cv::Vec3b pixel_dst(115, 82, 68);

	cv::transform(input_img, output_img, R.t());

	//std::cout << src << std::endl;
	//std::cout << dst << std::endl;



	//std::cout << "------------" << std::endl;

	//for (int i = 0; i < input_img.rows * input_img.cols; ++i)
	//{
	//	const cv::Vec3w pixel = input_img.at<cv::Vec3w>(i);
	//	const Eigen::Matrix<Type, 4, 1> v(pixel[0], pixel[1], pixel[2], 1);

	//	Eigen::Matrix<Type, 4, 1> tv = transform.matrix() * v;
	//	tv /= tv.w();

	//	//std::cout << (tv.head<3>()).transpose().cast<int>() << std::endl;

	//	output_img.at<cv::Vec3w>(i) = cv::Vec3w(tv[0], tv[1], tv[2]);
	//}

	imwrite("../data/color_checker/test_out.tif", output_img);

	return EXIT_SUCCESS;
}


#if 0
a <<
80, 60, 68,
164, 135, 155,
73, 102, 179,
70, 84, 68,
99, 107, 200,
108, 173, 198,
177, 108, 62,
43, 70, 194,
152, 66, 109,
57, 43, 116,
149, 177, 89,
191, 145, 61,
0, 45, 171,
79, 132, 87,
130, 38, 60,
201, 185, 61,
142, 62, 169,
3, 109, 181,
209, 215, 240,
178, 186, 222,
140, 150, 191,
95, 102, 137,
59, 63, 88,
30, 32, 45;

b <<
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
#endif