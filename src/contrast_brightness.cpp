#include <iostream>
#include <fstream>
#include <string>
#include "opencv2/highgui.hpp"



int main(int argc, char** argv)
{
	const char* window_name = "Contrast_&_Brightness";
	// Read original image 
	cv::Mat src = cv::imread(argv[1], CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);
	
	
	//if fail to read the image
	if (!src.data)
	{
		std::cout << "Error loading the image" << std::endl;
		return -1;
	}

	cv::Mat dst = cv::Mat::zeros(src.size(), src.type());

	// Create a window
	cv::namedWindow(window_name, CV_WINDOW_NORMAL);
	cv::resizeWindow(window_name, 1080 / 2, 1920 / 2);
	cv::moveWindow(window_name, 0, 0);

	//Create trackbar to change brightness
	int iSliderValue1 = 65535 / 2;
	cv::createTrackbar("Brightness", window_name, &iSliderValue1, 65535);

	//Create trackbar to change contrast
	int iSliderValue2 = 50;
	cv::createTrackbar("Contrast", window_name, &iSliderValue2, 100);


	while (true)
	{
		//Change the brightness and contrast of the image (For more infomation http://opencv-srf.blogspot.com/2013/07/change-contrast-of-image-or-video.html)
		int iBrightness = iSliderValue1 - (65535 / 2);
		double dContrast = iSliderValue2 / 50.0;
		
		src.convertTo(dst, -1, dContrast, (double)iBrightness);

		//show the brightness and contrast adjusted image
		cv::imshow(window_name, dst);

		// Wait until user press some key for 50ms
		int iKey = cv::waitKey(50);

		//if user press 'ESC' key
		if (iKey == 27)
		{
			break;
		}
	}

	return 0;
}


#if 0
int iSliderValue3 = 10;
createTrackbar("Exposure", "My Window", &iSliderValue3, 200);
float exposure = float(15);
float luminance = float(-0.5);

//double dExposure = iSliderValue3 / 10.0;

/// Do the operation new_image(i,j) = alpha*image(i,j) + beta
for (int y = 0; y < src.rows; y++)
{
	for (int x = 0; x < src.cols; x++)
	{
		//calculate exposure
		Vec3f input = wip.at<Vec3f>(y, x) * dExposure;

		//convert to YCbCr
		float Y = float(0.299*input[0] + 0.587 * input[1] + 0.114 * input[2]);
		float Cb = float(-0.1687 * input[0] + -0.3313 * input[1] + 0.5 * input[2] + 0.5);
		float Cr = float(0.5 * input[0] - 0.4187 * input[1] - 0.0813 * input[2] + 0.5);

		//calculate luminance
		Y = Y + luminance;

		//convert back to RGB
		float R = float(Y + 1.402 * (Cr - 0.5));
		float G = float(Y - 0.34414 * (Cb - 0.5) - 0.71414 * (Cr - 0.5));
		float B = float(Y + 1.772 * (Cb - 0.5));

		dst.at<Vec3w>(y, x)[0] = (ushort)(R * USHRT_MAX);
		dst.at<Vec3w>(y, x)[1] = (ushort)(G * USHRT_MAX);
		dst.at<Vec3w>(y, x)[2] = (ushort)(B * USHRT_MAX);

		//for (int c = 0; c < 3; c++)
		//{
		//	dst.at<Vec3w>(y, x)[c] = saturate_cast<uchar>(dContrast*(src.at<Vec3b>(y, x)[c]) + iBrightness);
		//}
	}
}
#endif