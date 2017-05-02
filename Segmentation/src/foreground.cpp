#include "foreground.h"

/*
   @brief: Find object contours from the image
   @param: source, source image
   @param: thresh, threshold for the algorithm
   @retval: image with the contour lines
*/
cv::Mat draw_contours(cv::Mat source, int thresh)
{
  cv::Mat canny_output;
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
	
  //Blur the image to reduce noise
  cv::blur( source, source, cv::Size(2, 2) );
  /// Detect edges using canny
  cv::Canny( source, canny_output, thresh, thresh*2, 3 );
  /// Find contours
  cv::findContours( canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

  /// Draw contours
  cv::Mat drawing = cv::Mat::zeros( canny_output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
		 cv::drawContours( drawing, contours, i, 0xFF, CV_FILLED, 8, hierarchy, 0, cv::Point() ); //Orig. width 2, CV_FILLED for fill
     }
	return drawing;
}

/*
   @brief:	Detect larger blobs from the binary fg mask
   @param:	src_img, image source
   @retval:	output, image file where the output is written
*/
cv::Mat draw_blobs(cv::Mat src_img)
{
	cv::Mat out_img;
	if(src_img.empty())
		return out_img;
	//Set up a vector with default parameters TODO Find better parameters
	cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create();

	//Find the blobs!
	std::vector<cv::KeyPoint> keypoints;
	detector->detect(src_img, keypoints);
	//Draw detected blobs as red circles
	//DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle correspons to the size of the blob
	cv::drawKeypoints(src_img, keypoints, out_img, cv::Scalar(0, 0, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	return out_img;
}
