/*
 * Bebop2Demo.cpp
 *
 *  Created on: Feb 1, 2019
 *      Author: slascos
 *
 *  This file demonstrate some basic OpenCV processing to achieve the following objectives:
 *  - have a primary window showing the unprocessed video stream from the dorne at frame rate
 *  - have a secondary window performing OpenCV processing on a second thread, potentially at
 *    much slower than frame rate
 *  - show how to capture keyboard events pressed when one one of the OpenCV windows is active
 */
#include <cstdlib> // for system()
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>

#include "wscDrone.h"
#include "Missions.h"
#include "OpenCVProcessing.h"

// Include the implementation information to use OpenCV as our VideoFrame provider
#include "VideoFrameOpenCV.h"
using VideoFrameGeneric = VideoFrameOpenCV;

using namespace std;
using namespace wscDrone;

#define NO_FLIGHT // uncomment thus to allow the drones to fly in this demo

// 'using' permits us to use things like 'Mat' instead of 'cv::Mat' all the time
using cv::Mat;
using cv::Rect;
using cv::Point;
using cv::Scalar;

// Global variables
// We need a vectory of drone instances, and a vector of frame objects for their streaming video
vector<shared_ptr<Bebop2>>            g_drones;
vector<shared_ptr<VideoFrameGeneric>> g_frames;

bool processingDone = true;      // flag used to indicated when the processing thread is done with a frame
bool shouldExit = false;         // flag used to indicate the program should exit.
int droneUnderManualControl = 0; // selects which drone is controlled manually by the keyboard

// FUNCTION PROTOTYPES
std::thread launchDisplayThread();         // a function to launch the primary display thread
void initDrones(vector<string> callsigns); // Takes in a vector of drone callsigns, and initializes each one
void openCVKeyCallbacks(const int key);    // process key presses from the OpenCV window

int main(int argc, char **argv)
{

    if (argc < 2) {
        cout << "\nERROR: You must specify the drone callsigns" << endl;
        cout << "./bebop2Swarm lone_wolf" << endl;
        cout << "./bebop2Swarm alpha" << endl;
        cout << "./bebop2Swarm alpha bravo" << endl;
        exit(EXIT_SUCCESS);
    }

    vector<string> args(argv, argv + argc); // Determine the requested drones from the command line
    initDrones(args); // Initialize drones

    // Check If any drones were initialized
    const int NUM_DRONES = g_drones.size();
    if (NUM_DRONES < 1) {
        std::cout << "No valid drones callsigns provided" << endl;
        exit(EXIT_SUCCESS);
    }

    // Launch the display thread
    auto displayThread = launchDisplayThread();

    // Start each drone one at a time
    for (int droneId = 0; droneId < NUM_DRONES; droneId++) {
        	startDrone(droneId);
    }

// If NO_FLIGHT is defined, the drones will not take off. This is helpful just to test
// video and move the drones around manually by hand.
#ifndef NO_FLIGHT

    // In order to get drones to do things simaltaneously, they need their own threads.
    // Both Alpha and Bravo will take off, execute mission1(), then land at the same time.
    std::thread alphaThread( [&]() {
        takeoffDrone(0);
        mission1(0);
        landDrone(0);
    });

    std::thread bravoThread( [&]() {
        takeoffDrone(1);
        mission1(1);
        landDrone(1);
    });


    // Wait for threads to complete
    if (alphaThread.joinable()) { alphaThread.join(); }
    if (bravoThread.joinable()) { bravoThread.join(); }
#endif

    if (displayThread.joinable()) { displayThread.join(); }
    return EXIT_SUCCESS;
}
 

void initDrones(vector<string> callsigns) {
    for (size_t i = 1; i < callsigns.size(); ++i) {
        if (callsigns[i] == "alpha") {
            g_frames.emplace_back(make_shared<VideoFrameGeneric>(BEBOP2_STREAM_HEIGHT, BEBOP2_STREAM_WIDTH));
            g_drones.emplace_back(make_shared<Bebop2>(Callsign::ALPHA, g_frames[i-1]));

        } else if (callsigns [i] == "lone_wolf") {
        	 g_frames.emplace_back(make_shared<VideoFrameGeneric>(BEBOP2_STREAM_HEIGHT, BEBOP2_STREAM_WIDTH));
            g_drones.emplace_back(make_shared<Bebop2>(Callsign::LONE_WOLF, g_frames[i-1]));


        } else if (callsigns [i] == "bravo") {
        	g_frames.emplace_back(make_shared<VideoFrameGeneric>(BEBOP2_STREAM_HEIGHT, BEBOP2_STREAM_WIDTH));
            g_drones.emplace_back(make_shared<Bebop2>(Callsign::BRAVO, g_frames[i-1]));


        } else if (callsigns [i] == "charlie") {
        	g_frames.emplace_back(make_shared<VideoFrameGeneric>(BEBOP2_STREAM_HEIGHT, BEBOP2_STREAM_WIDTH));
            g_drones.emplace_back(make_shared<Bebop2>(Callsign::CHARLIE, g_frames[i-1]));

        }
    }
}

// Create a function to launch the display thread. This will create two windows:
// 1) PRIMARY STREAMING WINDOW
//    - this divides the screen into quadrants and renders video from each drone into one of the quadrants
// 2) PROCESSING WINDOW
//    - this window is used to display processed video, information, etc. Processing is done on a separate thread
//      since it may be much slower than frame rate.
std::thread launchDisplayThread()
{
    std::thread displayThreadPtr = std::thread([]()
        {
        // Create windows
        constexpr unsigned SUBWINDOW_HEIGHT = 720/2;
        constexpr unsigned SUBWINDOW_WIDTH = 1280/2;
        char myString[250]; // used for text on screen
        int keypress = 0;   // used to capture user key-presses

        // Create a video streamign window. Video will be shown in quadrants, split into four for up to four drones
        cv::namedWindow("VIDEO Streaming", cv::WINDOW_NORMAL);
        cv::resizeWindow("VIDEO Streaming", 2*SUBWINDOW_WIDTH, 2*SUBWINDOW_HEIGHT);

        // Create a second video to display video processing related stuff
        cv::namedWindow("PROCESSING", cv::WINDOW_AUTOSIZE);

        // Create frames for storing our output images
        Mat streamingImage(2*SUBWINDOW_HEIGHT, 2*SUBWINDOW_WIDTH, CV_8UC3);
        shared_ptr<Mat> processingImagePtr = make_shared<Mat>();

        // Sub window positions, each drone gets a quadrant
        Rect subWindow[] = { Rect(0, 0, SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT),
                             Rect(SUBWINDOW_WIDTH,0,SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT),
                             Rect(0, SUBWINDOW_HEIGHT, SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT),
                             Rect(SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT, SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT)};

        while(!shouldExit) {
            constexpr unsigned OFFSET_X = 10;
            constexpr unsigned OFFSET_Y = 30;

            streamingImage = Scalar(0,0,0); // clear the output image
            {
                // Loop through each drone
                for (unsigned droneId=0; droneId<g_drones.size(); droneId++) {
                    lock_guard<mutex> lock(*(g_drones[droneId]->getVideoDriver()->getBufferMutex())); // lock the image buffer while rendering (reading) it

                    //-- Prepare the new frame
                    // Step 1 - Get the underlying OpenCV frame from the VideoFrameOpenCV Class
                    shared_ptr<Mat> frame = (dynamic_pointer_cast<VideoFrameOpenCV>(g_frames[droneId]))->getFrame();

                    // Step 2 - Convert the image from drone form RGB to BGR
                    Mat imageBGR;
                    cv::cvtColor(*frame, imageBGR, cv::COLOR_RGB2BGR); // convert for OpenCV BGR display

                    //-- Scale the video frame to fit in the STREAMING window
                    // Step 3 - Rescale to fit multiple drones in the window
                    Mat scaledImage;
                    cv::resize(imageBGR, scaledImage, cv::Size(SUBWINDOW_WIDTH, SUBWINDOW_HEIGHT), 0, 0, cv::INTER_CUBIC);
                    Mat subView = streamingImage(subWindow[droneId]);
                    scaledImage.copyTo(subView);

                    // Step 4 - print any onscreen text info
                    // Put the battery info on the screen for this drone
                    sprintf(myString, "Bat: %d", g_drones[droneId]->getBatteryLevel());
                    cv::putText(streamingImage, myString, Point(
                            subWindow[droneId].x + OFFSET_X,
                            subWindow[droneId].y + OFFSET_Y), cv::FONT_HERSHEY_DUPLEX, 1, Scalar(0,255,0));

                    //-- Perform primary image processing. With complex algorithms, this likely won't be able to keep up
                    // with streaming video so it will run at a lower rate.
                    if (processingDone == true) {  // only start a new frame when the old one is done

                        // First send the processed frame to the display (if it exists)
                        if (!processingImagePtr->empty()) {
                            cv::imshow("PROCESSING", *processingImagePtr); // Send the processed image to the window, dereference the pointer to get the Mat object
                        }
                        processingDone = false;  // clear the flag

                        // Deep copy the new frame to the processingImage buffer
                        imageBGR.copyTo(*processingImagePtr);

                        std::thread procThread(openCVProcessing, processingImagePtr, &processingDone); // Launch a new thread
                        procThread.detach(); // you must detach the thread
                    }

                }
            }

            cv::imshow("VIDEO Streaming", streamingImage);
            keypress = cv::waitKey(1);
            openCVKeyCallbacks(keypress);
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

case 112: // P - Take a picture with the selected drone, the download on a separate thread
    {
        g_drones[droneUnderManualControl]->getCameraControl()->capturePhoto();

        std::thread download(
                []() {
            string ipAddress = g_drones[droneUnderManualControl]->getIpAddress();
            ostringstream command;
            command << "wget -r --no-parent -nc -A '*.dng,*.jpg' -P ./drone_" <<  droneUnderManualControl;
            command << " -nd ftp://" << ipAddress << "/internal_000/Bebop_2/media/";
            cout << "Sending command: " << command.str() << endl;
            int ret = system(command.str().c_str()); // Send the command to a shell to execute
            if (ret != 0) { cout << "DOWNLOAD ERROR: returned " << ret << endl; }
        });
        download.detach();
        break;
    }


    case 32 :   // spacebar - LAND ALL DRONES
        {
            cout << "LANDING ALL DRONES!!!" << endl;
            for (unsigned droneId=0; droneId<g_drones.size(); droneId++) {
                g_drones[droneId]->getPilot()->land();
            }
        }
        break;

    case 201: // F12 KEY - EMERGENCY MOTOR CUT: THE DRONES WILL FALL!!!!
        cout << "EMERGENCY MOTOR CUTS!!! DRONES WILL FALL!!!" << endl;
        for (unsigned droneId=0; droneId<g_drones.size(); droneId++) {
            //stopDrone(droneId);
            g_drones[droneId]->getPilot()->CUT_THE_MOTORS();
        }
        break;
    case 118: // 'v'
        // restart the video pipeline
        g_drones[droneUnderManualControl]->getVideoDriver()->stop();
        waitSeconds(5);
        g_drones[droneUnderManualControl]->getVideoDriver()->start();
        break;

    case 49:   // 1
        droneUnderManualControl = 0;
        break;
    case 50:   // 2
        if (g_drones.size() >= 2) {
            droneUnderManualControl = 1;
        }
        break;
    case 51:   // 3
        if (g_drones.size() >= 3) {
            droneUnderManualControl = 2;
        }
        break;
    case 116: // 't'
        cout << "MANUAL: Taking off!" << endl;
        g_drones[droneUnderManualControl]->getPilot()->takeOff();
        break;
    case 108: // 't'
        cout << "MANUAL: Landing!" << endl;
        g_drones[droneUnderManualControl]->getPilot()->land();
        break;
    case 81:   // left arrow
        cout << "MANUAL: Left!" << endl;
        g_drones[droneUnderManualControl]->getPilot()->moveDirection(MoveDirection::LEFT);
        break;
    case 82:
        // up arrow
        cout << "MANUAL: Forward!" << endl;
        g_drones[droneUnderManualControl]->getPilot()->moveDirection(MoveDirection::FORWARD);
        break;
    case 83:   // right arrow
        cout << "MANUAL: Right!" << endl;
        g_drones[droneUnderManualControl]->getPilot()->moveDirection(MoveDirection::RIGHT);
        break;
    case 84:   // down arrow
        cout << "MANUAL: Down!" << endl;
        g_drones[droneUnderManualControl]->getPilot()->moveDirection(MoveDirection::BACK);
        break;
    case 43: // numeric "+"
        cout << "MANUAL: Asending!" << endl;
        g_drones[droneUnderManualControl]->getPilot()->moveDirection(MoveDirection::UP);
        break;
    case 45 : // numeric "-"
        cout << "MANUAL: Descending!" << endl;
        g_drones[droneUnderManualControl]->getPilot()->moveDirection(MoveDirection::DOWN);
        break;
    default:
        if (key > 0) {
            cout << "Unknown key pressed: " << key << endl;
        }
        break;
    }
}
