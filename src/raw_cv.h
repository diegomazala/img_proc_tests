
#ifndef _RAW_CV_H_
#define _RAW_CV_H_


#include "libraw.h"
#ifdef WIN32
#define snprintf _snprintf
#include <windows.h>
#else
#define O_BINARY 0
#endif

#ifdef USE_DNGSDK
#include "dng_host.h"
#include "dng_negative.h"
#include "dng_simple_image.h"
#include "dng_info.h"
#endif

#include "opencv2/imgproc.hpp"


class Rawcv
{
public:

	Rawcv()
	{
		setDefaultParameters();
		raw_processor.set_progress_handler(rawcv_progress_callback, (void *)" Image data passed");
	}


	~Rawcv()
	{
		unload();
	}

	void setBitsPerSample(int depth)
	{
		// Output 8 or 16 bit images
		raw_processor.imgdata.params.output_bps = (depth == 16 || depth == 8) ? depth : 8;
	}

	void setDefaultParameters()
	{
		// Forcing the Libraw to adjust sizes based on the capture device orientation
		raw_processor.adjust_sizes_info_only();

		// Output 16 bit images
		raw_processor.imgdata.params.output_bps = 16;

		// Set the gamma curve to Linear
		raw_processor.imgdata.params.gamm[0] = 1.0;
		raw_processor.imgdata.params.gamm[1] = 1.0;

		// Disable exposure correction (unless config "raw:auto_bright" == 1)
		raw_processor.imgdata.params.no_auto_bright = 0;
		// Use camera white balance if available
		raw_processor.imgdata.params.use_camera_wb = 1;
		// Turn off maximum threshold value (unless set to non-zero)
		raw_processor.imgdata.params.adjust_maximum_thr = 0.0f;
		// Set camera maximum value 
		raw_processor.imgdata.params.user_sat = 0;

		// Use embedded color profile. Values mean:
		// 0: do not use embedded color profile
		// 1 (default): use embedded color profile (if present) for DNG files
		//    (always), for other files only if use_camera_wb is set.
		// 3: use embedded color data (if present) regardless of white
		//    balance setting.
		raw_processor.imgdata.params.use_camera_matrix = 1;

		// By default we use sRGB primaries for simplicity
		// [0 - 5]  Output colorspace(raw, sRGB, Adobe, Wide, ProPhoto, XYZ)
		raw_processor.imgdata.params.output_color = 1;

		//raw_processor.imgdata.params.exp_correc = 1; // enable exposure correction
		//raw_processor.imgdata.params.exp_shift = exposure_value; // set exposure correction


		// Interpolation quality
		// note: LibRaw must be compiled with demosaic pack GPL2 to use demosaic
		// algorithms 5-9. It must be compiled with demosaic pack GPL3 for
		// algorithm 10 (AMAzE). If either of these packs are not included, it
		// will silently use option 3 - AHD.
		// "linear","VNG","PPG","AHD","DCB","AHD-Mod","AFD","VCD","Mixed","LMMSE","AMaZE","DHT","AAHD",
		raw_processor.imgdata.params.user_qual = 3;
	}

	bool process()
	{
		int ret;
		if (LIBRAW_SUCCESS != (ret = raw_processor.dcraw_process()))
		{
			fprintf(stderr, "Cannot do postpocessing on image: %s\n", libraw_strerror(ret));
			if (LIBRAW_FATAL_ERROR(ret))
				return false;
		}

		//process the image into memory buffer
		libraw_processed_image_t *image = raw_processor.dcraw_make_mem_image(&ret);
		if (LIBRAW_SUCCESS != ret)
		{
			fprintf(stderr, "Cannot unpack image into buffer\n");
			return false;
		}
		else
		{
			//create a Mat object by data obtained from LibRaw
			cv_image = cv::Mat(cv::Size(image->width, image->height), CV_16UC3, image->data, cv::Mat::AUTO_STEP);
			cv::cvtColor(cv_image, cv_image, CV_RGB2BGR);	//Convert RGB to BGR
			//cv::imwrite("../data/cv_image_raw.tif", cv_image);
			//cv::Mat cv_image_gamma(cv::Size(image->width, image->height), CV_16UC3);
			//cv_image.convertTo(cv_image_gamma, -1, 2.222, 45);
			//cv::imwrite("../data/cv_image_gamma_basic.tif", cv_image_gamma);
		}
		return true;
	}

	bool load(const std::string& input_raw_filename)
	{
		int ret = 0;

		ret = raw_processor.open_file(input_raw_filename.c_str(), 1);
		if (ret != LIBRAW_SUCCESS)
			return false;

		if ((ret = raw_processor.unpack()) != LIBRAW_SUCCESS)
		{
			fprintf(stderr, "Cannot unpack image: %s\n", libraw_strerror(ret));
			return false;
		}

		return true;
	}


	void unload()
	{
		raw_processor.recycle();
	}


	libraw_output_params_t& params()
	{
		return raw_processor.imgdata.params;
	}

	const cv::Mat& image() const
	{
		return cv_image;
	}



protected:


	static int rawcv_progress_callback(void *d, enum LibRaw_progress p, int iteration, int expected)
	{
		char *passed = (char *)(d ? d : "default string"); // data passed to callback at set_callback stage

														   //if (verbosity > 2) // verbosity set by repeat -v switches
		if (false) // verbosity set by repeat -v switches
			printf("CB: %s  pass %d of %d (data passed=%s)\n", libraw_strprogress(p), iteration, expected, passed);
		else if (iteration == 0) // 1st iteration of each step
			printf("Starting %s (expecting %d iterations)\n", libraw_strprogress(p), expected);
		else if (iteration == expected - 1)
			printf("%s finished\n", libraw_strprogress(p));

		return 0; // always return 0 to continue processing
	}


	//
	// attributes
	//
	LibRaw raw_processor;
	cv::Mat cv_image;
};






#endif // _RAW_CV_H_


