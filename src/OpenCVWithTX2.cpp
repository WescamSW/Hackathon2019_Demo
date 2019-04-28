/*
 * Bebop2Demo.cpp
 *
 *  Created on: Feb 1, 2019
 *      Author: slascos
 */
#include <unistd.h>
#include <iostream>
#include <thread>

#include "opencv2/opencv.hpp"
#include "OpenCVProcessing.h"

using namespace std;
using namespace cv;

// Global variables
bool processingDone = true;

// Function prototype
std::thread launchDisplayThread();
void openCVKeyCallbacks(const int key);


int main(int argc, char **argv)
{
    // Launch the display thread
    auto displayThread = launchDisplayThread();

    if (displayThread.joinable()) { displayThread.join(); } // wait for the display thread to exit

    return EXIT_SUCCESS;
}
 
std::thread launchDisplayThread()
{
    std::thread displayThreadPtr = std::thread([]()
        {
        // Create windows
        constexpr unsigned WINDOW_HEIGHT = 720;
        constexpr unsigned WINDOW_WIDTH = 1280;
        constexpr unsigned VIDEO_CAPTURE_HEIGHT = 480;
        constexpr unsigned VIDEO_CAPTURE_WIDTH = 640;
        char myString[250]; // used for text on screen
        int keypress = 0;   // used to capture user key-presses

        // Create a video streaming window. To show unprocessed video at full frame rate
        namedWindow("VIDEO Streaming", WINDOW_AUTOSIZE);

        // Create a second video to display processed video, may not be at full frame rate
        namedWindow("PROCESSING", WINDOW_AUTOSIZE);

        // Create frames for storing our output images
        Mat streamingImage(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);

        // Image processing will be done on another thread so this must be a heap object. Use a shared pointer!
        shared_ptr<Mat> processingImagePtr = make_shared<Mat>();

        // Create a video capture device
        cout << "Please wait, setting up capture device" << endl;
        VideoCapture videoCapture(0); // zero means USB camera device 0.

        // Check if camera opened successfully
        if(!videoCapture.isOpened()){
          cout << "Error opening video stream or file" << endl;
          return EXIT_FAILURE;
        }
        videoCapture.set(CAP_PROP_FRAME_WIDTH, VIDEO_CAPTURE_WIDTH);
        videoCapture.set(CAP_PROP_FRAME_HEIGHT, VIDEO_CAPTURE_HEIGHT);
        cout << "Done setting up camera" << endl;

        while(true) { // infinite loop
            constexpr unsigned TEXT_OFFSET_X = 10;
            constexpr unsigned TEXT_OFFSET_Y = 30;

            {
            	Mat capturedFrame;
            	// Capture the next video frame
            	videoCapture >> capturedFrame;

            	if (capturedFrame.empty()) { continue; } // skip to next capture frame

            	// Scale the captured image to the desired window size
            	Mat scaledImage;
            	resize(capturedFrame, streamingImage, Size(WINDOW_WIDTH, WINDOW_HEIGHT), 0, 0, INTER_CUBIC);

            	// Put some example text on the streaming image
            	sprintf(myString, "WINDOW SIZE is %d x %d", WINDOW_WIDTH, WINDOW_HEIGHT);
				cv::putText(streamingImage, myString, Point(
						TEXT_OFFSET_X,
						TEXT_OFFSET_Y), FONT_HERSHEY_DUPLEX, 1, Scalar(0,255,0));

                //-- Perform primary image processing. With complex algorithms, this likely won't be able to keep up
				// with streaming video so it will run at a lower rate.
				if (processingDone == true) {  // only start a new frame when the old one is done

					if (!processingImagePtr->empty()) {
						cv::imshow("PROCESSING", *processingImagePtr); // Send the processed image to the window, dereference the pointer to get the Mat object
					}
                    processingDone = false;  // clear the flag
                    *processingImagePtr = capturedFrame; // update to use the newly captured frame
                    std::thread procThread(openCVProcessing, processingImagePtr, &processingDone); // Launch a new thread
                    procThread.detach(); // you must detach the thread
				}
            }

            cv::imshow("VIDEO Streaming", streamingImage); // Send the streaming image to the window

            keypress = cv::waitKey(1); // update the display
            openCVKeyCallbacks(keypress); // If we want to use keyboard inputs for anything...
        }
    }
    );
    return displayThreadPtr;
}


void openCVKeyCallbacks(const int key)
{

}



