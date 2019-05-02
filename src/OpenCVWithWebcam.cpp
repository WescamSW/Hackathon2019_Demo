/*
 * OpenCVWithWebcam.cpp
 *
 *  Created on: Feb 1, 2019
 *      Author: slascos
 *
 *  This file demonstrate some basic OpenCV processing to achieve the following objectives:
 *  - have a primary window showing the unprocessed video stream from a camera at frame rate
 *  - have a secondary window performing OpenCV processing on a second thread, potentially at
 *    much slower than frame rate
 *  - show how to capture keyboard events pressed when one one of the OpenCV windows is active
 */
#include <unistd.h>
#include <iostream>
#include <thread>

#include "opencv2/opencv.hpp"
#include "OpenCVProcessing.h"

using namespace std;
using cv::Mat;
using cv::Point;
using cv::Scalar;

// Global variables
bool processingDone = true; // flag used to indicated when the processing thread is done with a frame
bool shouldExit = false;     // flag used to indicate the program should exit.

// Function prototypes
std::thread launchDisplayThread();
void openCVKeyCallbacks(const int key);

// On the TX2, the onboard camera is /dev/video0 but doesn't work with OpenCV
// If you plug in a USB webcam it will be /dev/video1
// In the VM, the USB webcam would likely be /dev/video0
//const int videoDeviceId = 0; // For non-TX2 environment
const int videoDeviceId = 1; // for TX2


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
        cv::namedWindow("VIDEO Streaming", cv::WINDOW_AUTOSIZE);

        // Create a second video to display processed video, may not be at full frame rate
        cv::namedWindow("PROCESSING", cv::WINDOW_AUTOSIZE);

        // Create frames for storing our output images
        Mat streamingImage(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);

        // Image processing will be done on another thread so this must be a heap object. Use a shared pointer!
        shared_ptr<Mat> processingImagePtr = make_shared<Mat>();

        // Create a video capture device
        cout << "Please wait, setting up capture device" << endl;
        cv::VideoCapture videoCapture(videoDeviceId);

        // Check if camera opened successfully
        if(!videoCapture.isOpened()){
          cout << "Error opening video stream or file" << endl;
          return;
        }
        videoCapture.set(cv::CAP_PROP_FRAME_WIDTH,  VIDEO_CAPTURE_WIDTH);
        videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, VIDEO_CAPTURE_HEIGHT);
        cout << "Done setting up camera" << endl;

        Mat grayImage;

        while(!shouldExit) { // loop until request for exit
            constexpr unsigned TEXT_OFFSET_X = 10;
            constexpr unsigned TEXT_OFFSET_Y = 30;

            {
            	Mat capturedFrame;
            	// Capture the next video frame
            	videoCapture >> capturedFrame;

            	if (capturedFrame.empty()) { continue; } // skip to next capture frame

            	// Scale the captured image to the desired window size
            	Mat scaledImage;
            	cv::resize(capturedFrame, streamingImage, cv::Size(WINDOW_WIDTH, WINDOW_HEIGHT), 0, 0, cv::INTER_CUBIC);

            	// Put some example text on the streaming image
            	sprintf(myString, "WINDOW SIZE is %d x %d", WINDOW_WIDTH, WINDOW_HEIGHT);
				cv::putText(streamingImage, myString, Point(
						TEXT_OFFSET_X,
						TEXT_OFFSET_Y), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,255,0));

                //-- Perform primary image processing. With complex algorithms, this likely won't be able to keep up
				// with streaming video so it will run at a lower rate.
				if (processingDone == true) {  // only start a new frame when the old one is done

				    // First send the processed frame to the display (if it exists)
		            if (!processingImagePtr->empty()) {
		                cv::imshow("PROCESSING", *processingImagePtr); // Send the processed image to the window, dereference the pointer to get the Mat object
		            }
                    processingDone = false;  // clear the flag

                    // Deep copy the newly capture frame to the processingImage buffer
                    capturedFrame.copyTo(*processingImagePtr);

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
    switch(key) {
    case 27 : // ESCAPE key
        shouldExit = true;
        break;
    default :
        if (key >= 0) { // cv::waitKey() returns -1 when no key pressed
            cout << "Unmapped key pressed: " << key << endl;
        }
    }

}



