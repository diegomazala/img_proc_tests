#ifndef _BACKGROUND_SUBTRACTION_H_
#define _BACKGROUND_SUBTRACTION_H_


//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/bgsegm.hpp"
#include <opencv2/highgui.hpp>

#include <iostream>


class BackgroundSubtraction
{

public:

	BackgroundSubtraction()
	{
		setDefaultParameters();
	}


	~BackgroundSubtraction()
	{

	}


	void setDefaultParameters()
	{
		save_intermediate = true;
		save_result = true;
		output_filename = "out";
		output_extension = ".tif";
		morphological_kernel_size = 35;
	}


	void setResultFileName(
		const std::string& folder,
		const std::string& base_name, 
		const std::string& file_extension)
	{
		output_folder = folder;
		output_filename = base_name;
		output_extension = file_extension;
	}

	void saveIntermediateFiles(bool save)
	{
		save_intermediate = save;
	}

	void setMorphologicalKernelSize(int kernel_size)
	{
		morphological_kernel_size = kernel_size;
	}

	void saturateAbs(bool sat)
	{
		saturate_abs = sat;
	}


	void process(const cv::Mat& frame_bg_gray, const cv::Mat& frame_fg_gray)
	{
		std::stringstream ss;

		const uint8_t bits_per_sample = (frame_bg_gray.depth() == CV_16U || frame_bg_gray.depth() == CV_16S) ? 16 : 8;
		const uint32_t max_value = (uint32_t)pow(2, bits_per_sample);

		//
		// image subtraction
		//
		std::cout << "Subtracting background ... ";
		int64 start_time = cv::getTickCount();
		cv::Mat frame_abs_diff;
		cv::absdiff(frame_fg_gray, frame_bg_gray, frame_abs_diff);
		if (saturate_abs)
			frame_abs_diff.convertTo(frame_abs_diff, -1, 2, 0);	// saturate
		std::cout << double(cv::getTickCount() - start_time) / cv::getTickFrequency() << std::endl;
		//
		if (save_intermediate)
		{
			ss.str(std::string());
			ss.clear();
			ss << output_folder << output_filename << "_s0_abs_subtraction" << output_extension;
			std::cout << "Saving: " << ss.str() << std::endl;
			cv::imwrite(ss.str(), frame_abs_diff);

			//ss.str(std::string());
			//ss.clear();
			//ss << output_folder << output_filename << "_s00_abs_subtraction" << output_extension;
			//frame_abs_diff.convertTo(frame_abs_diff, -1, 2, 0);
			//cv::imwrite(ss.str(), frame_abs_diff);

		}



		//
		// binary threshold
		//
		std::cout << "Binary threshold ...       ";
		start_time = cv::getTickCount();
		cv::Mat binary_threshold;
		if (bits_per_sample == 16)
			frame_abs_diff.convertTo(frame_abs_diff, CV_8U, 1.0 / 256.0);
		cv::threshold(frame_abs_diff, binary_threshold, float(max_value) * 0.1f, max_value, cv::THRESH_BINARY);
		std::cout << double(cv::getTickCount() - start_time) / cv::getTickFrequency() << std::endl;
		//if (save_intermediate)
		//{
		//	ss.str(std::string());
		//	ss.clear();
		//	ss << output_folder << output_filename << "_s1_binary_threshold" << output_extension;
		//	std::cout << "Saving: " << ss.str() << std::endl;
		//	cv::imwrite(ss.str(), binary_threshold);
		//}


		//
		// morphological filters
		//
		std::cout << "Morphological filters ...  ";
		start_time = cv::getTickCount();
		cv::Mat erode_img, dilate_img;
		cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS,
			cv::Size(2 * morphological_kernel_size + 1, 2 * morphological_kernel_size + 1),
			cv::Point(morphological_kernel_size, morphological_kernel_size));
#if 0
		cv::dilate(binary_threshold, dilate_img, element);
		cv::erode(dilate_img, erode_img, element);
#else
		cv::dilate(binary_threshold, dilate_img, element);
		cv::erode(dilate_img, erode_img, element);
		cv::erode(erode_img, dilate_img, element);
		cv::dilate(dilate_img, erode_img, element);
#endif
		std::cout << double(cv::getTickCount() - start_time) / cv::getTickFrequency() << std::endl;
		if (save_intermediate)
		{
			ss.str(std::string());
			ss.clear();
			ss << output_folder << output_filename << "_s2_dilate_erode" << output_extension;
			std::cout << "Saving: " << ss.str() << std::endl;
			cv::imwrite(ss.str(), erode_img);
		}



		//
		// Floodfill from point (0, 0)
		//
		std::cout << "Floodfill ...              ";
		start_time = cv::getTickCount();
		cv::Mat floodfill = erode_img.clone();
		cv::floodFill(floodfill, cv::Point(0, 0), cv::Scalar(255));
		cv::floodFill(floodfill, cv::Point(floodfill.cols - 1 , floodfill.rows - 1), cv::Scalar(255));
		//
		// Invert floodfilled image
		cv::Mat floodfill_inv;
		bitwise_not(floodfill, floodfill_inv);
		std::cout << double(cv::getTickCount() - start_time) / cv::getTickFrequency() << std::endl;
		//
		//if (save_intermediate)
		//{
		//	ss.str(std::string());
		//	ss.clear();
		//	ss << output_folder << output_filename << "_s3_floodfill" << output_extension;
		//	std::cout << "Saving : " << ss.str() << std::endl;
		//	cv::imwrite(ss.str(), floodfill_inv);
		//}



		//
		// Logical operator OR to get mask.
		//
		std::cout << "Computing mask ...         ";
		start_time = cv::getTickCount();
		result_mask = (erode_img | floodfill_inv);
		
		std::cout << double(cv::getTickCount() - start_time) / cv::getTickFrequency() << std::endl;
		if (save_result)
		{
			ss.str(std::string());
			ss.clear();
			ss << output_folder << output_filename << "_mask" << output_extension;
			std::cout << "Saving : " << ss.str() << std::endl;
			cv::imwrite(ss.str(), result_mask);

		}
	}




	void compose(const cv::Mat& rgb_input, cv::Mat& rgba_output)
	{
#if 1
		const uint8_t bits_per_sample = (rgb_input.depth() == CV_16U || rgb_input.depth() == CV_16S) ? 16 : 8;

		cv::Mat alpha(result_mask.rows, result_mask.cols, CV_16UC1);
		if (bits_per_sample == 16)
			result_mask.convertTo(alpha, CV_16UC1, 255);	// 8bits to 16bits

		cv::Mat src_imgs[] = { rgb_input, alpha };
		int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
		rgba_output = cv::Mat(rgb_input.rows, rgb_input.cols, ((bits_per_sample == 16) ? CV_16UC4 : CV_8UC4));
		cv::mixChannels(src_imgs, 2, &(rgba_output), 1, from_to, 4);

		//
		// Save result
		//
		if (save_result)
		{
			std::stringstream ss;
			ss << output_folder << output_filename << "_rgba" << output_extension;
			std::cout << "Saving : " << ss.str() << std::endl;
			cv::imwrite(ss.str(), rgba_output);
		}
		
#else
		//
		// Apply mask to input fg image
		//
		//cv::Mat segmentation;
		//rgb_input.copyTo(segmentation, result_mask);

		cv::Mat alpha(rgb_input.rows, rgb_input.cols, CV_16UC1);
		result_mask.convertTo(alpha, CV_16UC1, 255);	// 8bits to 16bits

		cv::Mat src_imgs[] = { rgb_input, alpha };
		int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
		rgba_output = cv::Mat(rgb_input.rows, rgb_input.cols, CV_16UC4);
		cv::mixChannels(src_imgs, 2, &(rgba_output), 1, from_to, 4);
#endif
	}


	const cv::Mat& resultMask() const
	{
		return result_mask;
	}

	



protected:

	cv::Mat result_mask;
	bool save_intermediate			= false;
	bool save_result				= true;
	std::string output_folder		= "";
	std::string output_filename			= "out";
	std::string output_extension	= ".tif";
	int morphological_kernel_size	= 35;
	bool saturate_abs				= false;
};


#endif // _BACKGROUND_SUBTRACTION_H_

