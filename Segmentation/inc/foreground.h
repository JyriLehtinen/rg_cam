#ifndef FOREGROUND_H
#define FOREGROUND_H

#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <opencv2/opencv.hpp>

#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>

#include <stdio.h>

using namespace cv;
/*
   @brief: Find object contours from the image
   @param: source, source image
   @param: thresh, threshold for the algorithm
   @retval: image with the contour lines
*/

Mat draw_contours(Mat source, int thresh);

/*
   @brief:	Detect larger blobs from the binary fg mask
   @param:	src_img, image source
   @retval:	output, image file where the output is written
*/
Mat draw_blobs(Mat src_img);

#endif //FOREGROUND_H
