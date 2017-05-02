#ifndef PROJECTION_H
#define PROJECTION_H

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

/*
	@brief: This function saves the 3D reference model into the model vectors
	@param: model, the 3D vectors, units in cm, origin at the center of the court
	@retval: None
*/
void construct_model(std::vector<cv::Point3f> *model);

/*
   @brief: Construct the camera intrisic parameters. TODO Add a rough calibration function?
   @param: im, image taken with the camera in use
   @param: matrix, the output matrix
   @rturn: 0 on success
*/
int construct_camera(cv::Mat im, cv::Mat *matrix, cv::Mat *_dist_coeffs);

/*
   @brief: Project points into the image based on solvePnP output
   @param: _camera, the camera intrisic parameters matrix
   @param: _distorsion, camera distorsion matrix
   @param: _rvec, rotation matrix
   @param: _tvec, translation matrix
*/
cv::Mat project_points(cv::Mat _camera, cv::Mat _distorsion, cv::Mat _rvec, cv::Mat _tvec, cv::Mat *image);

#endif //PROJECTION_H
