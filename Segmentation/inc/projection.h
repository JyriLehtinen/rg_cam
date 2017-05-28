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

using namespace cv;
using namespace std;
/*
	@brief: This function saves the 3D reference model into the model vectors
	@param: model, the 3D vectors, units in cm, origin at the center of the court
	@retval: None
*/
void construct_model(vector<Point3f> *model);

/*
   @brief: Construct the camera intrisic parameters. TODO Add a rough calibration function?
   @param: im, image taken with the camera in use
   @param: matrix, the output matrix
   @rturn: 0 on success
*/
int construct_camera(Mat im, Mat *matrix, Mat *_dist_coeffs);

/*
   @brief: Project points into the image based on solvePnP output
   @param: _camera, the camera intrisic parameters matrix
   @param: _distorsion, camera distorsion matrix
   @param: _rvec, rotation matrix
   @param: _tvec, translation matrix
*/
Mat project_points(Mat _camera, Mat _distorsion, Mat _rvec, Mat _tvec, Mat *image);


/*
   @brief: Create a transformation matrix we pass to the Blender API
   @param: rvec, Rotation vector solved with solvePnP()
   @param: tvec, Translation vector solved with solvePnP()
   @param: dst, pointer to output matrix
   @retval: 0 on success
*/
int construct_transformation(Mat rvec, Mat tvec, Mat* dst);

/*
   @brief: Cut the edge out from the original video, leaving the court only
   @param: _camera, the camera intrisic parameters matrix
   @param: _distorsion, camera distorsion matrix
   @param: _rvec, rotation matrix
   @param: _tvec, translation matrix
   @param: image, pointer to the image we're editing
   @retval: Cropped image
*/
Mat crop_image(Mat _camera, Mat _distorsion, Mat _rvec, Mat _tvec, Mat image);
#endif //PROJECTION_H
