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
#include <iostream>

using namespace std;
using namespace cv;

/* Private defines */
#define LEARN_TIME		7 		//How long weighed learning is running
#define BG_HISTORY		1000	//BG subtraction algorithm history length
#define BG_THRESH		30		//BG subtraction threshold

/* Global variables */
vector<Point2f> court_lines;
vector<Point3f> model_points;
Mat tvec, rvec, camera_matrix, dist_coeffs;

static void help()
{
 printf("\nDo background segmentation, especially demonstrating the use of cvUpdateBGStatModel().\n"
"Learns the background at the start and then segments.\n"
"Learning is togged by the space key. Will read from file or camera\n"
"Usage: \n"
"			./bgfg_segm [--camera]=<use camera, if this key is present>, [--file_name]=<path to movie file> \n\n");
}

/** @function thresh_callback */
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

/* @brief:	Detect larger blobs from the binary fg mask
   @param:	src_img, image source
   @retval:	output, image file where the output is written
*/
Mat draw_blobs(Mat src_img) //TODO Doesn't really work here
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

/*
   	@brief: Adjust the background subtraction learning rate to weigh the beginning more.
   			Assuming that video starts with only background in the picture
	@param: fps, video frames per second
	@param: count, frame counter
	@ret: 	0 on success
*/
int adjust_learning(int fps, int count, double *rate) //TODO Add more gradual change to the learning instead of step.
{
	if((count/fps) < LEARN_TIME)
	{
		*rate = 0.0075;
	}
	else
	{
		*rate = 0.0001;
	}

	return 0;
}
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

	cout << "Camera Matrix " << endl << camera_matrix << endl ;
}


void on_mouse( int e, int x, int y, int d, void *ptr )
{
	if(e == EVENT_LBUTTONDOWN)
	{
		if(court_lines.size() < 6)
		{
			court_lines.push_back(Point2f(float(x), float(y)));
			cout << x << " " << y << endl;
		}
		else
		{
			cout << "Solvin' 'n' shoite!" << endl;

			if(	solvePnP(model_points, court_lines, camera_matrix, dist_coeffs, rvec, tvec, false, CV_ITERATIVE))
			{
				cout << "court solvePnP succeeded?!" << endl;
				cout << "rvec: " << rvec << endl <<  "tvec: " << tvec << endl;
			}
		}
	}
	return;
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

const char* keys =
{
    "{c   camera   |         | use camera or not}"
    "{m   method   |mog2     | method (knn or mog2) }"
    "{s   smooth   |         | smooth the mask }"
    "{fn  file_name|../data/tree.avi | movie file        }"
	"{p   pic      |         | new background picture }"
	"{vid bg_vid   |         | new background video   }"
	"{cn  contours |         | detect contours  }"
	"{bl  blobs    |         | detect blobs, not working}"
};

//this is a sample for foreground detection functions
int main(int argc, const char** argv)
{
    help();

    CommandLineParser parser(argc, argv, keys);
    bool useCamera = parser.has("camera");
    bool smoothMask = parser.has("smooth");
	bool replacePic = parser.has("pic");
	bool replaceVid = parser.has("bg_vid");
	bool detectContours = parser.has("contours");
	bool detectBlobs = parser.has("blobs");
	
	string picture = parser.get<string>("pic");
	string bg_video = parser.get<string>("vid");

    string file = parser.get<string>("file_name");
    string method = parser.get<string>("method");
    VideoCapture cap;

	double* rate = (double *)malloc(sizeof(double));
	int fps;
	long frm_cnt = 0;

	Mat image;
	VideoCapture bg_cap;
    bool update_bg_model = true;

	if( replacePic )
	{
		printf("Background replacement active\n");
		image = imread(picture,1);
		if(image.empty())
		{
			printf("Error reading image, seems empty :/ \n");
			replacePic = false;
		}
	}

	if(replaceVid)
	{
		printf("Background video active\n");
		bg_cap.open(bg_video.c_str());
	}

    if( useCamera )
        cap.open(0);
    else
        cap.open(file.c_str());

    parser.printMessage();

    if( !cap.isOpened() )
    {
        printf("can not open camera or video file\n");
        return -1;
    }
	
	if(!bg_cap.isOpened() && replaceVid)
	{
		printf("Can not open background video\n");
		replaceVid = false;
	}

	fps = cap.get(CV_CAP_PROP_FPS);
	cout << "Frames per second using video.get(CV_CAP_PROP_FPS) : " << fps << endl;
 
    namedWindow("image", WINDOW_NORMAL);
    //namedWindow("foreground mask", WINDOW_NORMAL);
    namedWindow("foreground image", WINDOW_NORMAL);
	//namedWindow("mean background image", WINDOW_NORMAL);
	setMouseCallback("image", on_mouse, NULL);
	if(detectContours)
		namedWindow("Contours", WINDOW_NORMAL);
	if(detectBlobs)
		namedWindow("Blobs", WINDOW_NORMAL);
    
	Ptr<BackgroundSubtractor> bg_model = method == "knn" ?
            createBackgroundSubtractorKNN().dynamicCast<BackgroundSubtractor>() :
            createBackgroundSubtractorMOG2(BG_HISTORY, BG_THRESH, false).dynamicCast<BackgroundSubtractor>();

    Mat img0, img, fgmask, fgimg;
	Mat bg_img0, bg_img, areas, blob_img;
	
	bool camera_calibrated = false;
	
	construct_model(&model_points);
    for(;;)
    {
        cap >> img0;

        if( img0.empty() )
            break;

		img = img0;
		if(!camera_calibrated)
		{
			construct_camera(img, &camera_matrix, &dist_coeffs);
			camera_calibrated = true;
		}
        if( fgimg.empty() )
          fgimg.create(img.size(), img.type());

        //update the model
		adjust_learning(fps, frm_cnt++, rate);
        bg_model->apply(img, fgmask, update_bg_model ? *rate : 0); //-1 learning rate means automatic adjustment, 0.005 seemed good for the clip

        if( smoothMask )
        {
            GaussianBlur(fgmask, fgmask, Size(1, 1), 3.5, 3.5);
            threshold(fgmask, fgmask, 30, 255, THRESH_BINARY);
        }


		if(replacePic)
		{
			resize(image, bg_img, img0.size(), 0, 0, INTER_LINEAR);
		}


		else if(replaceVid)
		{
			bg_cap >> bg_img0;
			if(bg_img0.empty())
			{
				bg_cap.set(CV_CAP_PROP_POS_FRAMES, 0);
				bg_cap >> bg_img0;
			}
			if(!bg_img0.empty())
				resize(bg_img0, bg_img, img0.size(), 0, 0, INTER_LINEAR);
		}

        fgimg = Scalar::all(0);
		if(replacePic || replaceVid)
			bg_img.copyTo(fgimg);
        img.copyTo(fgimg, fgmask);

        Mat bgimg;
        bg_model->getBackgroundImage(bgimg);
		
		if(detectContours)
		{
			areas = draw_contours(fgmask, 300);
			imshow("Contours", areas);
		}

		if(detectBlobs)
		{
			blob_img = draw_blobs(fgmask);
			imshow("Blobs", blob_img);
		}
		if(!update_bg_model)
			img = project_points(camera_matrix, dist_coeffs, rvec, tvec, &img);

        imshow("image", img);
        //imshow("foreground mask", fgmask);
        imshow("foreground image", fgimg);
        /*if(!bgimg.empty())
          imshow("mean background image", bgimg );*/
        char k = (char)waitKey(10); //Original value is 30
        if( k == 27 ) break;
        if( k == ' ' )
        {
            update_bg_model = !update_bg_model;
			project_points(camera_matrix, dist_coeffs, rvec, tvec, &img);

            if(update_bg_model)
                printf("Background update is on\n");
            else
                printf("Background update is off\n");
        }
    }

	free(rate);
    return 0;
}
