
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

#include <vector>
#include <iostream>

class Rawcv
{
public:

	Rawcv();
	~Rawcv();

	void setDefaultOptions();
	
	bool process();

	bool load(const std::string& input_raw_filename);
	
	void unload();

	libraw_output_params_t& params()
	{
		return raw_processor.imgdata.params;
	}

protected:

	LibRaw raw_processor;
	cv::Mat cv_image;
};

#endif // _RAW_CV_H_
