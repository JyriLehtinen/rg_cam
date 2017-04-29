#include "projection.h"
#include <iostream>

using namespace cv;
using namespace std;

/*
	@brief: This function saves the 3D reference model into the model vectors
	@param: model, the 3D vectors, units in cm, origin at the center of the court
	@reval: None
*/
void construct_model(vector<Point3f> *model)
{
	//Starting from the "upper left" service corner across the net, going clocwise
	(*model).push_back(Point3f(0.0f, 0.0f, 640.0f));
	(*model).push_back(Point3f(823.0f, 0.0f, 640.0f));
	(*model).push_back(Point3f(823.0f, 0.0f, 0.0f));
	(*model).push_back(Point3f(823.0f, 0.0f, -640.0f));
	(*model).push_back(Point3f(0.0f, 0.0f, -640.0f));
	(*model).push_back(Point3f(0.0f, 0.0f, 0.0f));
}

/*
   @brief: Construct the camera intrisic parameters. TODO Add a rough calibration function?
   @param: im, image taken with the camera in use
   @param: matrix, the output matrix
   @rturn: 0 on success
*/
int construct_camera(Mat im, Mat *matrix, Mat *_dist_coeffs)
{
	/*
	// Camera internals
    double focal_length = im.cols; // Approximate focal length.
    Point2d center = Point2d(im.cols/2,im.rows/2);
    *matrix = (Mat_<double>(3,3) << focal_length, 0, center.x, 0 , focal_length, center.y, 0, 0, 1);
    *_dist_coeffs = Mat::zeros(4,1,DataType<double>::type); // Assuming no lens distortion
*/	
	// Values for Sony Xperia Z3 Compact, default horizontal video
	*matrix = (Mat_<double>(3,3) <<
			1.88389429e+03, 0.00000000e+00, 9.55374691e+02,
			0.00000000e+00, 1.89846625e+03, 5.44115238e+02,
			0.00000000e+00, 0.00000000e+00, 1.00000000e+00);
			
	*_dist_coeffs = (Mat_<double>(5,1) <<
			-1.71533585e-01, 5.12063874e+00, 8.05474932e-05, -3.19247362e-03, -3.58186431e+01);

	cout << "Camera Matrix " << endl << *matrix << endl ;
}

/*
   @brief: Project points into the image based on solvePnP output
   @param: _camera, the camera intrisic parameters matrix
   @param: _distorsion, camera distorsion matrix
   @param: _rvec, rotation matrix
   @param: _tvec, translation matrix
*/
Mat project_points(Mat _camera, Mat _distorsion, Mat _rvec, Mat _tvec, Mat *image=NULL)
{
	vector<Point2f> image_points;
	vector<Point3f> posts;
	
	posts.push_back(Point3f(-137.2f, 0.0f, 1189.0f));
	posts.push_back(Point3f(960.2f, 0.0f, 1189.0f));
	posts.push_back(Point3f(-228.6f, 0.0f, 0.0f));
	posts.push_back(Point3f(1051.6f, 0.0f, 0.0f));
	projectPoints(posts, _rvec, _tvec, _camera, _distorsion, image_points);	
	//cout << "Projected points: " << image_points << endl;
	if(image == NULL || (*image).empty())
		return *image;
	else
	{
		for(int i=0; i < image_points.size(); i++)
		{
			circle(*image, image_points[i], 10, Scalar(0,0,255), 4);
		}
	}
	
	return *image;
}
