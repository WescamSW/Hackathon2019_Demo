/*
 * Bebop2Demo.cpp
 *
 *  Created on: Feb 1, 2019
 *      Author: slascos
 */
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>

#include "wscDrone.h"

#include "Definitions.h" // Defines WESCAM_VIDEO or OPENCV_VIDEO
#include "Missions.h"

using namespace std;
using namespace wscDrone;

// Global variables
vector<shared_ptr<Bebop2>>            g_drones;
vector<shared_ptr<VideoFrameGeneric>> g_frames;

/* 
    Takes drone callsigns given by user and creates an instance of the following types:
    vector<shared_ptr<Semaphore>>
    vector<unique_ptr<mutex>>            
    vector<shared_ptr<VideoFrame>>      
    vector<shared_ptr<Bebop2>>          
*/
void initDrones(vector<string> callsigns);


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

    //Check If any drones were initialized
    const int NUM_DRONES = g_drones.size();
    if (NUM_DRONES < 1) {
        std::cout << "No valid drones callsigns provided" << endl;
        exit(EXIT_SUCCESS);
    }

    // Launch the display thread
    auto displayThread = VideoFrameGeneric::launchDisplayThread();

    // Start each drone one at a time
    for (int droneId = 0; droneId < NUM_DRONES; droneId++) {
        	startDrone(droneId);
    }

    //Here we can start a thread that will process the video using opencv calls

    int droneId = 0;

    takeoffDrone(droneId);
    for (int i=0; i<4; i++) {
      mission1(droneId);
    }
    //waitSeconds(30);
    landDrone(droneId);

    while(true) {}
    exit(0);


    std::thread alpha( [&]() {
        takeoffDrone(0);
        setFlightAltitude(0, 2.5f);
        waitSeconds(5);
        missionTriange(0);
        waitSeconds(5);
        landDrone(0);
    });

    std::thread bravo( [&]() {
        waitSeconds(5);
        takeoffDrone(1);
        setFlightAltitude(1, 1.5f);
        missionTriange(1);
        landDrone(1);
    });


    // Wait for threads to complete
    if (alpha.joinable()) { alpha.join(); }
    if (bravo.joinable()) { bravo.join(); }

    printf("THREADS COMPLETE\n");
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
