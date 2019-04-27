/*
 * VideoOpenCV.cpp
 *
 *  Created on: Apr 15, 2019
 *      Author: Reg Marr
 *
 *  Description: This module is used to handle requests for image data
 *  using opencv calls. It also provides a method for displaying video
 *  and images using opencv (launchDisplayThread)
 */
#include <vector>
#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "Missions.h"
#include "wscDrone.h"
#include "VideoOpenCV.h"

using namespace std;
using namespace wscDrone;
using namespace cv;

extern vector<shared_ptr<Bebop2>>     g_drones;
extern vector<shared_ptr<VideoFrame>> g_frames;

int droneUnderControl = 0;


VideoFrameOpenCV::VideoFrameOpenCV(unsigned height, unsigned width)
: VideoFrame(height, width)
{
    m_frame = std::make_shared<cv::Mat>(height,width,CV_8UC3);
}

VideoFrameOpenCV::VideoFrameOpenCV(unsigned height, unsigned width, const uint8_t * frame)
: VideoFrame(height, width)
{
    m_frame = make_shared<Mat>(height, width, CV_8UC3, (void * )frame);
}

char *VideoFrameOpenCV::getRawPointer()
{
    return reinterpret_cast<char *>(m_frame->ptr());
}
size_t VideoFrameOpenCV::getFrameSizeBytes()
{
    return m_frame->total() * m_frame->elemSize();
}

unsigned VideoFrameOpenCV::getHeight() {
    unsigned height = 0;
    if (m_frame) {
        height =  m_frame->rows;
    }
    return height;
}

unsigned VideoFrameOpenCV::getWidth() {
    unsigned width = 0;
    if (m_frame) {
        width = m_frame->cols;
    }
    return width;
}

void openCVProcessing(Mat &inputImage)
{

}

void openCVKeyCallbacks(const int key)
{
    switch(key) {
    case 32:   // spacebar - LAND ALL DRONES
        {
            for (unsigned droneId=0; droneId<g_drones.size(); droneId++) {
                stopDrone(droneId);
            }
        }
        break;
    case 49:   // 1
        droneUnderControl = 0;
        break;
    case 50:   // 2
        if (g_drones.size() >= 2) {
            droneUnderControl = 1;
        }
        break;
    case 51:   // 3
        if (g_drones.size() >= 3) {
            droneUnderControl = 2;
        }
        break;
    case 116: // 't'
        cout << "MANUAL: Taking off!" << endl;
        g_drones[droneUnderControl]->getPilot()->takeOff();
        break;
    case 108: // 't'
        cout << "MANUAL: Landing!" << endl;
        g_drones[droneUnderControl]->getPilot()->land();
        break;
    case 81:   // left arrow
        cout << "MANUAL: Left!" << endl;
        g_drones[droneUnderControl]->getPilot()->moveDirection(MoveDirection::LEFT);
        break;
    case 82:
        // up arrow
        cout << "MANUAL: Forward!" << endl;
        g_drones[droneUnderControl]->getPilot()->moveDirection(MoveDirection::FORWARD);
        break;
    case 83:   // right arrow
        cout << "MANUAL: Right!" << endl;
        g_drones[droneUnderControl]->getPilot()->moveDirection(MoveDirection::RIGHT);
        break;
    case 84:   // down arrow
        cout << "MANUAL: Down!" << endl;
        g_drones[droneUnderControl]->getPilot()->moveDirection(MoveDirection::BACK);
        break;
    case 43: // numeric "+"
        cout << "MANUAL: Asending!" << endl;
        g_drones[droneUnderControl]->getPilot()->moveDirection(MoveDirection::UP);
        break;
    case 45 : // numeric "-"
        cout << "MANUAL: Descending!" << endl;
        g_drones[droneUnderControl]->getPilot()->moveDirection(MoveDirection::DOWN);
        break;
    default:
        if (key > 0) {
            cout << "Unknown key pressed: " << key << endl;
        }
        break;
    }
}

// Create a function to launch the display thread
std::shared_ptr<std::thread> VideoFrameOpenCV::launchDisplayThread()
{
    std::shared_ptr<std::thread> displayThreadPtr = make_shared<std::thread>([]()
        {
        // Create windows
        constexpr unsigned SUBWINDOW_HEIGHT = 720/2;
        constexpr unsigned SUBWINDOW_WIDTH = 1280/2;
        char myString[250]; // used for text on screen
        int keypress = 0;   // used to capture user key-presses

        // Create a video streamign window. Video will be shown in quadrants, split into four for up to four drones
        namedWindow("VIDEO Streaming", WINDOW_NORMAL);
        resizeWindow("VIDEO Streaming", 2*SUBWINDOW_WIDTH, 2*SUBWINDOW_HEIGHT);

        // Create a second video to display video processing related stuff
        namedWindow("PROCESSING", WINDOW_AUTOSIZE);

        // Create frames for storing our output images
        Mat streamingImage(2*SUBWINDOW_HEIGHT, 2*SUBWINDOW_WIDTH, CV_8UC3);
        Mat processingImage;

        // Sub window positions, each drone gets a quadrant
        Rect subWindow[] = { Rect(0, 0, SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT),
                             Rect(SUBWINDOW_WIDTH,0,SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT),
							 Rect(0, SUBWINDOW_HEIGHT, SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT),
							 Rect(SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT, SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT)};

        while(true) {
            constexpr unsigned OFFSET_X = 10;
            constexpr unsigned OFFSET_Y = 30;

            streamingImage = Scalar(0,0,0); // clear the output image
            {
            	// Loop through each drone
                for (unsigned droneId=0; droneId<g_drones.size(); droneId++) {
                    //std::lock_guard<std::mutex> lock(*g_bufferGuards[droneId]); // lock the image buffer while rendering (reading) it
                    lock_guard<mutex> lock(*(g_drones[droneId]->getVideoDriver()->getBufferMutex()));

                    //-- Prepare the new frame
                    // Step 1 - Get the underlying OpenCV frame from the Video Frame Class
                    shared_ptr<Mat> frame = (dynamic_pointer_cast<VideoFrameOpenCV>(g_frames[droneId]))->getFrame();

                    // Step 2 - Convert the image from RGB to BGR
                    Mat imageBGR;
                    cvtColor(*frame, imageBGR, COLOR_RGB2BGR); // convert for OpenCV BGR display

                    //-- Scale the video frame to fit in the STREAMING window
                    // Step 3 - Rescale to fit multiple drones in the window
                    Mat scaledImage;
                    resize(imageBGR, scaledImage, Size(SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT), 0, 0, INTER_CUBIC);
                    Mat subView = streamingImage(subWindow[droneId]);
                    scaledImage.copyTo(subView);

                    // Step 4 - print any onscreen text info
                    // Put the battery info on the screen for this drone
                    sprintf(myString, "Bat: %d", g_drones[droneId]->getBatteryLevel());
                    cv::putText(streamingImage, myString, Point(
                    		subWindow[droneId].x + OFFSET_X,
							subWindow[droneId].y + OFFSET_Y), FONT_HERSHEY_DUPLEX, 1, Scalar(0,255,0));

                    //-- Perform primary processing. In this case, simply display the first drones video as fullscreen in
                    // the processing window.
                    if (droneId == 0) {
						processingImage = imageBGR;
						openCVProcessing(processingImage);
                    }

                }
            }

            cv::imshow("VIDEO Streaming", streamingImage);
            cv::imshow("PROCESSING", processingImage);
            keypress = cv::waitKey(1);
            openCVKeyCallbacks(keypress);
        }
    }
    );
    return displayThreadPtr;
}
