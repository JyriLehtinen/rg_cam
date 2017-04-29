# rg_cam
ME310 RG Cam project repository
ILLUSTRATION WORKS ONLY WHEN YOU VIEW THE RAW FILE!

## INSTALLATION

git clone https://github.com/JyriLehtinen/rg_cam
cd Segmentation
mkdir build
cd build
cmake ..
make

And tah-dah! RG_Cam is compiled and ready to run!

## ADDITIONAL STEPS
To get some video materials to test with, download tapiola.mp4 from https://drive.google.com/drive/folders/0B38pdAj5NrE-b056RE15T3BhbDQ

## USAGE

Simple way to try it out is this:
./RG_Cam --fn=../../videos/tapiola.mp4 -p=../../pics/court.jpg

There are additional arguments that can be used, but that's usually enough to try most things.
To try out the solvePNP, you have to make 6 clicks (4 should work, but now it's configured for 6) with the mouse in the "image" window at specific points (Right now in specific order, though that could be sorted)
It starts from the upper left corner of the service line and singles sideline behind the net, proceeding clockwise to the right side,
then crossing point of net and singles sideline, etc. Here's a crude illustration:
-----------------------------
|  |                    |   |
|  |                    |   |
|  |                    |   |
|  1--------------------2  |
|  |          |         |   |
|  |          |         |   |
|  |          |         |   |
===6====================3=====
|  |          |         |   |
|  |          |         |   |
|  5----------|---------4   |
|  |                    |   |
|  |                    |   |
|  |                    |   |
-----------------------------
****CAMERA SOMEWHERE HERE****
            
7th click on the window will calculate the rotation and translation vectors for the camera. The intrisic camera parameters are currently for Sony Xperia Z3 compact camera, don't know how it is with other phones.
Anyway, all cameras can easily be calibrated for good results.

After the solvePnP() is done, pressing spacebar will create projection points for the far corners of the court. This works for all camera positions at the back of the court.
That's it for now!
