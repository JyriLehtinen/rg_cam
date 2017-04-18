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
using namespace std;
using namespace cv;

/* Private defines */
#define LEARN_TIME		7 		//How long weighed learning is running
#define BG_HISTORY		1000	//BG subtraction algorithm history length
#define BG_THRESH		22		//BG subtraction threshold

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
    	drawContours( drawing, contours, i, 0xFF, 2, 8, hierarchy, 0, Point() );
     }
	return drawing;
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

const char* keys =
{
    "{c   camera   |         | use camera or not}"
    "{m   method   |mog2     | method (knn or mog2) }"
    "{s   smooth   |         | smooth the mask }"
    "{fn  file_name|../data/tree.avi | movie file        }"
	"{p   pic      |         | new background picture }"
	"{vid bg_vid   |         | new background video   }"
	"{pd  people   |         | detect people   }"
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
	bool detectPeople = parser.has("people");

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
    namedWindow("foreground mask", WINDOW_NORMAL);
    namedWindow("foreground image", WINDOW_NORMAL);
	namedWindow("mean background image", WINDOW_NORMAL);

	if(detectPeople)
		namedWindow("Contours", WINDOW_NORMAL);
	
    Ptr<BackgroundSubtractor> bg_model = method == "knn" ?
            createBackgroundSubtractorKNN().dynamicCast<BackgroundSubtractor>() :
            createBackgroundSubtractorMOG2(BG_HISTORY, BG_THRESH, false).dynamicCast<BackgroundSubtractor>();

    Mat img0, img, fgmask, fgimg;
	Mat bg_img0, bg_img, areas;

    for(;;)
    {
        cap >> img0;

        if( img0.empty() )
            break;

        resize(img0, img, Size(640, 640*img0.rows/img0.cols), INTER_LINEAR);

        if( fgimg.empty() )
          fgimg.create(img.size(), img.type());

        //update the model
		adjust_learning(fps, frm_cnt++, rate);
        bg_model->apply(img, fgmask, update_bg_model ? *rate : 0); //-1 learning rate means automatic adjustment, 0.005 seemed good for the clip

        if( smoothMask )
        {
            GaussianBlur(fgmask, fgmask, Size(11, 11), 3.5, 3.5);
            threshold(fgmask, fgmask, 10, 255, THRESH_BINARY);
        }


		if(replacePic)
		{
			resize(image, bg_img, Size(640, 640*img0.rows/img0.cols), INTER_LINEAR);
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
				resize(bg_img0, bg_img, Size(640, 640*img0.rows/img0.cols), INTER_LINEAR);
		}

        fgimg = Scalar::all(0);
		if(replacePic || replaceVid)
			bg_img.copyTo(fgimg);
        img.copyTo(fgimg, fgmask);

        Mat bgimg;
        bg_model->getBackgroundImage(bgimg);
		
		if(detectPeople)
		{
			areas = draw_contours(fgmask, 300);
			imshow("Contours", areas);
		}

        imshow("image", img);
        imshow("foreground mask", fgmask);
        imshow("foreground image", fgimg);
        if(!bgimg.empty())
          imshow("mean background image", bgimg );

        char k = (char)waitKey(10); //Original value is 30
        if( k == 27 ) break;
        if( k == ' ' )
        {
            update_bg_model = !update_bg_model;
            if(update_bg_model)
                printf("Background update is on\n");
            else
                printf("Background update is off\n");
        }
    }

	free(rate);
    return 0;
}
