#include "foreground.h"

using namespace cv;
using namespace std;
/*
   @brief: Find object contours from the image
   @param: source, source image
   @param: thresh, threshold for the algorithm
   @retval: image with the contour lines
*/
Mat draw_contours(Mat source, int thresh)
{
  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
	
  //Blur the image to reduce noise
  blur( source, source, Size(2, 2) );
  /// Detect edges using canny
  Canny( source, canny_output, thresh, thresh*2, 3 );
  /// Find contours
  findContours( canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Draw contours
  Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
    	drawContours( drawing, contours, i, 0xFF, CV_FILLED, 8, hierarchy, 0, Point() ); //Orig. width 2, CV_FILLED for fill
     }
	return drawing;
}

/*
   @brief:	Detect larger blobs from the binary fg mask
   @param:	src_img, image source
   @retval:	output, image file where the output is written
*/
Mat draw_blobs(Mat src_img)
{
	Mat out_img;
	if(src_img.empty())
		return out_img;
	//Set up a vector with default parameters TODO Find better parameters
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create();

	//Find the blobs!
	std::vector<KeyPoint> keypoints;
	detector->detect(src_img, keypoints);
	//Draw detected blobs as red circles
	//DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle correspons to the size of the blob
	drawKeypoints(src_img, keypoints, out_img, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	return out_img;
}
