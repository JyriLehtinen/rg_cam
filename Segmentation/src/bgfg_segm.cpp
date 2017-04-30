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
