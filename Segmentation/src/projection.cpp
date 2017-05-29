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
	/*
	//Starting from the "upper left" service corner across the net, going clocwise
	(*model).push_back(Point3f(-411.5f, 0.0f, 640.0f));
	(*model).push_back(Point3f(411.5f, 0.0f, 640.0f));
	(*model).push_back(Point3f(411.5f, 0.0f, 0.0f));
	(*model).push_back(Point3f(411.5f, 0.0f, -640.0f));
	(*model).push_back(Point3f(-411.5f, 0.0f, -640.0f));
	(*model).push_back(Point3f(-411.5f, 0.0f, 0.0f));
	*/
	(*model).push_back(Point3f(-411.5f, 640.0f, 0.0f));
	(*model).push_back(Point3f(411.5f, 640.0f, 0.0f));
	(*model).push_back(Point3f(411.5f, 0.0f, 0.0f));
	(*model).push_back(Point3f(411.5f, -640.0f, 0.0f));
	(*model).push_back(Point3f(-411.5f, -640.0f, 0.0f));
	(*model).push_back(Point3f(-411.5f, 0.0f, 0.0f));
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
	/*
	// Values for Sony Xperia Z3 Compact, default horizontal video
	*matrix = (Mat_<double>(3,3) <<
			1.88389429e+03, 0.00000000e+00, 9.55374691e+02,
			0.00000000e+00, 1.89846625e+03, 5.44115238e+02,
			0.00000000e+00, 0.00000000e+00, 1.00000000e+00);
			
	*_dist_coeffs = (Mat_<double>(5,1) <<
			-1.71533585e-01, 5.12063874e+00, 8.05474932e-05, -3.19247362e-03, -3.58186431e+01);
	*/
	
	//Values for Logitech Live! Chat HD -Webcamera (RMS = 0.55607779207):
	*matrix = (Mat_<double>(3,3) <<
			1.14978204e+03, 0.00000000e+00, 6.73382667e+02,
			0.00000000e+00, 1.15080886e+03, 3.00863716e+02,
			0.00000000e+00, 0.00000000e+00, 1.00000000e+00);

	*_dist_coeffs = (Mat_<double>(5, 1) <<
			0.12454721, -0.95423951, -0.01841303,  0.00211118,  1.84403588);

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
	
	posts.push_back(Point3f(-548.7f, 1189.0f, 0));
	posts.push_back(Point3f(548.7f, 1189.0f, 0));
	posts.push_back(Point3f(0.0f, 0.0f, 91.4f));
//	posts.push_back(Point3f(-228.6f, 0.0f, 0.0f));
//	posts.push_back(Point3f(1051.6f, 0.0f, 0.0f));
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

/*
   @brief: Create a transformation matrix we pass to the Blender API
   @param: rvec, Rotation vector solved with solvePnP()
   @param: tvec, Translation vector solved with solvePnP()
   @param: dst, pointer to output matrix
   @retval: 0 on success
*/
int construct_transformation(Mat rvec, Mat tvec, Mat* dst)
{
	Mat rotation; 
	Rodrigues(rvec, rotation);

	for(unsigned int row=0; row<3; ++row)
	{
	   for(unsigned int col=0; col<3; ++col)
	   {
		  dst->at<double>(row, col) = rotation.at<double>(row, col);
	   }
	   dst->at<double>(row, 3) = tvec.at<double>(row, 0);
	}

	for(int col=0; col < 3; ++col)
	{
		dst->at<double>(3, col) = 0.0f;
	}

	dst->at<double>(3, 3) = 1.0f;

	return 0;
}

/*
   @brief: Cut the edge out from the original video, leaving the court only
   @param: _camera, the camera intrisic parameters matrix
   @param: _distorsion, camera distorsion matrix
   @param: _rvec, rotation matrix
   @param: _tvec, translation matrix
   @param: image, pointer to the image we're editing
   @retval: mask for cropping
*/
Mat crop_image(Mat _camera, Mat _distorsion, Mat _rvec, Mat _tvec, Mat image)
{
	/* Our region of interest in the video is the original court,
	   slightly extended on the sides and elevated to get the original net in the picture.
	   The following points are just outside the (doubles) court lines
	*/
	Mat mask(image.size(), CV_8U, Scalar(0)); 

	vector<Point2f> image_points;
	vector<Point3f> world_corners;
	
	world_corners.push_back(Point3f(-648.7f, 0, 30.0f)); //Left net post
	world_corners.push_back(Point3f(-548.7f, 1189.0f, 60.0f)); //"From upper left corner of doubles, 0,5m up.
	world_corners.push_back(Point3f(548.7f, 1189.0f, 60.0f)); //"From upper right corner of doubles, 0,5m up.
	world_corners.push_back(Point3f(648.7f, 0, 30.0f)); //Right net post

	projectPoints(world_corners, _rvec, _tvec, _camera, _distorsion, image_points);	//Calculate where these points would be in the image TODO Check whether points are behind camera or not
//cout << image_points << endl;

	Point left_edge = Point(0, (image_points[1].y - ((image_points[1].y - image_points[0].y)/(image_points[1].x - image_points[0].x) * image_points[1].x)));
	Point right_edge = Point(image.cols, ( image_points[2].y + (image_points[3].y - image_points[2].y)/(image_points[3].x - image_points[2].x) * (image.cols - image_points[2].x) ) ); 

	Point crop_points[1][6];

	crop_points[0][0] = Point(0, image.rows); //Bottom left corner
	crop_points[0][1] = Point(left_edge); //Left edge of the diagonal line
	crop_points[0][2] = Point(image_points[1] ); //Upper left back corner 
	crop_points[0][3] = Point(image_points[2] ); //Upper right back corner
	crop_points[0][4] = Point(right_edge); //Right edge of the diagonal line
	crop_points[0][5] = Point(image.cols, image.rows); //Bottom right corner

	const Point* ppt[1] = { crop_points[0] };
	int npt[] = { 6 };

	fillPoly( mask, ppt, npt, 1, Scalar( 255, 255, 255 ), 8 );

	
	//Return the mask for the cutout
	return mask;
}
