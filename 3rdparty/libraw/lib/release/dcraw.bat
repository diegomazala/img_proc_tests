:: -c float-num       Set adjust maximum threshold (default 0.75)
:: -v        Verbose: print progress messages (repeated -v will add verbosity)
:: -w        Use camera white balance, if possible
:: -a        Average the whole image for white balance
:: -A <x y w h> Average a grey box for white balance
:: -r <r g b g> Set custom white balance
:: +M/-M     Use/don't use an embedded color matrix
:: -C <r b>  Correct chromatic aberration
:: -P <file> Fix the dead pixels listed in this file
:: -K <file> Subtract dark frame (16-bit raw PGM)
:: -k <num>  Set the darkness level
:: -S <num>  Set the saturation level
:: -R <num>  Set raw processing options to num
:: -n <num>  Set threshold for wavelet denoising
:: -H [0-9]  Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild)
:: -t [0-7]  Flip image (0=none, 3=180, 5=90CCW, 6=90CW)
:: -o [0-5]  Output colorspace (raw,sRGB,Adobe,Wide,ProPhoto,XYZ)
:: -o file   Output ICC profile
:: -p file   Camera input profile (use \'embed\' for embedded profile)
:: -j        Don't stretch or rotate raw pixels
:: -W        Don't automatically brighten the image
:: -b <num>  Adjust brightness (default = 1.0)
:: -q N      Set the interpolation quality:
::           0 - linear, 1 - VNG, 2 - PPG, 3 - AHD, 4 - DCB
::           5 - modified AHD,6 - AFD (5pass), 7 - VCD, 8 - VCD+AHD, 9 - LMMSE
::           10-AMaZE
:: -h        Half-size color image (twice as fast as \"-q 0\")
:: -f        Interpolate RGGB as four colors
:: -m <num>  Apply a 3x3 median filter to R-G and B-G
:: -s [0..N-1] Select one raw image from input file
:: -4        Linear 16-bit, same as \"-6 -W -g 1 1
:: -6        Write 16-bit linear instead of 8-bit with gamma
:: -g pow ts Set gamma curve to gamma pow and toe slope ts (default = 2.222 4.5)
:: -T        Write TIFF instead of PPM
:: -G        Use green_matching() filter
:: -B <x y w h> use cropbox
:: -F        Use FILE I/O instead of streambuf API
:: -timing   Detailed timing report
:: -fbdd N   0 - disable FBDD noise reduction (default), 1 - light FBDD, 2 - full
:: -dcbi N   Number of extra DCD iterations (default - 0)
:: -dcbe     DCB color enhance
:: -eeci     EECI refine for mixed VCD/AHD (q=8)
:: -esmed N  Number of edge-sensitive median filter passes (only if q=8)
:: /"-amazeca  Use AMaZE chromatic aberrations refine (only if q=10)
:: -acae <r b>Use chromatic aberrations correction // modifJD
:: -aline <l> reduction of line noise
:: -aclean <l c> clean CFA
:: -agreen <g> equilibrate green
:: -aexpo <e p> exposure correction
:: -apentax4shot enables merge of 4-shot pentax files
:: -apentax4shotorder 3102 sets pentax 4-shot alignment order
:: -dbnd <r g b g> debanding
:: -mmap     Use mmap()-ed buffer instead of plain FILE I/O
:: -mem	   Use memory buffer instead of FILE I/O
:: -disars   Do not use RawSpeed library
:: -disinterp Do not run interpolation step
:: -dsrawrgb1 Disable YCbCr to RGB conversion for sRAW (Cb/Cr interpolation enabled)
:: -dsrawrgb2 Disable YCbCr to RGB conversion for sRAW (Cb/Cr interpolation disabled)
:: -disadcf  Do not use dcraw Foveon code even if compiled with demosaic-pack-GPL2
:: -dngsdk   Use Adobe DNG SDK for DNG decode
:: -dngflags N set DNG decoding options to value N
dcraw_emu.exe -timing -F -6 -W -g 1 1 -j -o 1 -w -v -T tadeu_color_chart.CR2