/*
 * Missions.cpp
 *
 *  Created on: Feb 7, 2019
 *      Author: slascos
 */
#include <iostream>
#include <memory>
#include <cmath>
#include <thread>

#include "wscDrone.h"
#include "Missions.h"

using namespace std;
using namespace wscDrone;

// Global variables
extern vector<shared_ptr<Bebop2>> g_drones;

// Create some helpful convenience aliases
using ControlPtr       = shared_ptr<DroneController>;
using CameraControlPtr = shared_ptr<CameraControl>;
using PilotPtr         = shared_ptr<Pilot>;
using VideoDriverPtr   = shared_ptr<VideoDriver>;

constexpr int WAIT_SHORT = 5;
constexpr int WAIT_MED   = 7;
constexpr int WAIT_LONG  = 10;

void changeAltitude(int droneId, float heightMeters)
{
    PilotPtr pilot   = g_drones[droneId]->getPilot();
    pilot->moveRelativeMetresRestricted(0.0, 0.0, -heightMeters);
}

void startDrone(int droneId)
{

    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();

    control->start();
    cout << "Done CONTROL START" << endl;
    waitSeconds(1);
    video->start();
    cout << "Done VIDEO START" << endl;
    camera->setForward();
    waitSeconds(1);
    cout << "Done drone start" << endl;
}

void takeoffDrone(int droneId) {

    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();
    g_drones[droneId]->getPilot()->takeOff();
}

void landDrone(int droneId)
{
    // Mission shutdown
    g_drones[droneId]->getCameraControl()->setForward();
    g_drones[droneId]->getPilot()->land();
}

void stopDrone(int droneId)
{
    // Mission shutdown
    g_drones[droneId]->getCameraControl()->setForward();
    g_drones[droneId]->getPilot()->land();
    waitSeconds(5);
    g_drones[droneId]->getVideoDriver()->stop();
    g_drones[droneId]->getDroneController()->stop();
}

void missionDance2()
{

    takeoffDrone(0);
    takeoffDrone(1);
    //takeoffDrone(2);

    thread alpha, bravo, charlie;

    alpha   = thread(mission1, 0);
    bravo   = thread(mission1, 1);
    //charlie = thread(mission1, 2);

    if (alpha.joinable())   { alpha.join(); }
    if (bravo.joinable())   { bravo.join(); }
   // if (charlie.joinable()) { charlie.join(); }

    landDrone(0);
    landDrone(1);
  //  landDrone(2);

}

void missionDance()
{
    const float STEP_DISTANCE = 2.0f;
    const float TURN_ANGLE =  -120.0f;

    takeoffDrone(0);
    takeoffDrone(1);
    //takeoffDrone(2);

    thread alpha, bravo, charlie;

    alpha   = thread(missionTriange, 0);
    bravo   = thread(missionTriange, 1);
    //charlie = thread(missionTriange, 2);

    if (alpha.joinable())   { alpha.join(); }
    if (bravo.joinable())   { bravo.join(); }
    //if (charlie.joinable()) { charlie.join(); }


    // Align drones
    alpha = thread( [&](){
        g_drones[0]->getPilot()->moveRelativeMetresRestricted(STEP_DISTANCE, STEP_DISTANCE, -2.0f);
        g_drones[0]->getPilot()->setHeading(2*TURN_ANGLE);
    } );
    bravo = thread( [&](){
        g_drones[1]->getPilot()->moveRelativeMetresRestricted(0, STEP_DISTANCE, -1.0f);
        g_drones[1]->getPilot()->setHeading(TURN_ANGLE);
    } );

    if (alpha.joinable())   { alpha.join(); }
    if (bravo.joinable())   { bravo.join(); }

    // Run the triangle again
    alpha   = thread(missionTriange, 0);
    bravo   = thread(missionTriange, 1);
    //charlie = thread(missionTriange, 2);

    if (alpha.joinable())   { alpha.join(); }
    if (bravo.joinable())   { bravo.join(); }
    //if (charlie.joinable()) { charlie.join(); }

}



void setFlightAltitude(int droneId, float heightMeters)
{
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    pilot->moveRelativeMetresRestricted(0.0, 0.0, -heightMeters);
}

// Look around with the camera, then move forward 1 metre, than back 1 metre.
//void mission1(shared_ptr<Bebop2> bebop2Ptr)
void mission1(int droneId)
{
    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();
//
//    camera->setTiltPan(17.0f, 45.0f); // Look up and to the right
//    waitSeconds(5);
//    camera->setTiltPan(-17.0f, -45.0f); // Look down and to the left
//    waitSeconds(5);
//    camera->setForward();
//    waitSeconds(5);

    pilot->moveRelativeMetresRestricted(2.0, 0.00, 0.0); // Move forward 1 metre
    pilot->moveRelativeMetresRestricted(-2.0, 0.0, 0.0); // move backward 1 meter

    //pilot->moveRelativeMetresRestricted(0,  2.0, 0.0); // Move forward 1 metre
    //pilot->moveRelativeMetresRestricted(0, -2.0, 0.0); // move backward 1 meter
}

// Move in a 10-metre square pattern, turning such that drone is always facing inwards,
// and moving (sliding) to the right.
void missionBox(int droneId)
{
    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();

    const float STEP = 4.0f;
    const float HALF_STEP = 2.0f;

    const int WAIT_TIME = 10;
    waitSeconds(WAIT_SHORT);
    pilot->moveRelativeMetresRestricted(0.0, 0.0, -2.5); waitSeconds(WAIT_TIME);  // More up 2.5 meters
    pilot->moveRelativeMetresRestricted(0.0, HALF_STEP, 0.0);  waitSeconds(WAIT_TIME); // Move right 5 metres
    pilot->setHeading(LEFT_90_DEGREES);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, STEP, 0.0); waitSeconds(WAIT_TIME); // Move right 10 metre
    pilot->setHeading(LEFT_90_DEGREES);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, STEP, 0.0); waitSeconds(WAIT_TIME); // Move right 10 metre
    pilot->setHeading(LEFT_90_DEGREES);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, STEP, 0.0); waitSeconds(WAIT_TIME); // Move right 10 metre
    pilot->setHeading(LEFT_90_DEGREES);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, HALF_STEP, 0.0);  waitSeconds(WAIT_TIME); // Move right 5 metres
}

void missionBoxInv(int droneId)
{
    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();

    const int WAIT_TIME = 10;
    const float STEP = 4.0f;
    const float HALF_STEP = 2.0f;

    waitSeconds(WAIT_SHORT);
    pilot->moveRelativeMetresRestricted(0.0, 0.0, -1.0); waitSeconds(WAIT_TIME);  // More up 2.5 meters
    pilot->moveRelativeMetresRestricted(0.0, -HALF_STEP, 0.0);  waitSeconds(WAIT_TIME); // Move left 5 metres
    pilot->setHeading(RIGHT_90_DEGREES);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, -STEP, 0.0); waitSeconds(WAIT_TIME); // Move left 10 metre
    pilot->setHeading(RIGHT_90_DEGREES);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, -STEP, 0.0); waitSeconds(WAIT_TIME); // Move left 10 metre
    pilot->setHeading(RIGHT_90_DEGREES);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, -STEP, 0.0); waitSeconds(WAIT_TIME); // Move left 10 metre
    pilot->setHeading(RIGHT_90_DEGREES);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, -HALF_STEP, 0.0);  waitSeconds(WAIT_TIME); // Move left 5 metres
}

void missionTriange(int droneId)
{
    PilotPtr         pilot   = g_drones[droneId]->getPilot();

    const int WAIT_TIME = 0;
    const float STEP_DISTANCE = 2.0f;
    const float TURN_ANGLE =  -120.0f;
    //waitSeconds(WAIT_SHORT);

    pilot->moveRelativeMetresRestricted(0.0, STEP_DISTANCE, 0.0);  waitSeconds(WAIT_TIME); // Move right 5 metres
    pilot->setHeading(TURN_ANGLE);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, STEP_DISTANCE, 0.0); waitSeconds(WAIT_TIME); // Move left 10 metre
    pilot->setHeading(TURN_ANGLE);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
    pilot->moveRelativeMetresRestricted(0.0, STEP_DISTANCE, 0.0); waitSeconds(WAIT_TIME); // Move left 10 metre
    pilot->setHeading(TURN_ANGLE);             waitSeconds(WAIT_TIME); // rotate counter-clockwise 90
}

// Fly a T pattern, facing the same orientation the entire time.
void missionTForward(int droneId)
{
    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();

    waitSeconds(WAIT_SHORT);
    pilot->moveRelativeMetresRestricted(10.0, 0.0, 0.0); waitSeconds(WAIT_LONG); // More forward 10
    pilot->moveRelativeMetresRestricted(0.0, 2.0, 0.0);  waitSeconds(WAIT_LONG); // move right 2
    pilot->moveRelativeMetresRestricted(0.0, -4.0, 0.0); waitSeconds(WAIT_LONG); // Move left 4
    pilot->moveRelativeMetresRestricted(0.0, 2.0, 0.0);  waitSeconds(WAIT_LONG); // Move right 2
    pilot->moveRelativeMetresRestricted(-10.0, 0.0, 0.0); waitSeconds(WAIT_LONG); // More back 10
}

// Follow a T pattern, by turning such that all movements are forward orientation
void missionTTurn(int droneId)
{
    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();

    waitSeconds(WAIT_LONG);
    pilot->moveRelativeMetresRestricted(0.0, 0.0, -0.5); waitSeconds(WAIT_LONG);  // More up 2.5 meters
    pilot->moveRelativeMetresRestricted(10.0, 0.0, 0.0); waitSeconds(WAIT_LONG);  // More forward 10
    pilot->setHeading(RIGHT_90_DEGREES); waitSeconds(WAIT_LONG);            // rotate clockwise 90
    pilot->moveRelativeMetresRestricted(2.0, 0.0, 0.0);  waitSeconds(WAIT_LONG);  // move forward 2
    pilot->setHeading(RIGHT_180_DEGREES); waitSeconds(WAIT_LONG);                 // rotate clockwise 180
    pilot->moveRelativeMetresRestricted(4.0, 0.0, 0.0); waitSeconds(WAIT_LONG);  // Move forward 4
    pilot->setHeading(LEFT_180_DEGREES); waitSeconds(WAIT_LONG);                 // rotate counterclockwise 180
    pilot->moveRelativeMetresRestricted(2.0, 0.0, 0.0);  waitSeconds(WAIT_LONG);  // move forward 2
    pilot->setHeading(RIGHT_90_DEGREES); waitSeconds(WAIT_SHORT);            // rotate clockwise 90
    pilot->moveRelativeMetresRestricted(10.0, 0.0, 0.0); waitSeconds(WAIT_LONG); // More forward 10
    pilot->setHeading(RIGHT_180_DEGREES); waitSeconds(WAIT_LONG);                 // rotate clockwise 180
}



// Go forward, turned around, then come back
void missionTurnBack(int droneId)
{
    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();

    waitSeconds(WAIT_SHORT);
    pilot->moveRelativeMetresRestricted(10.0, 0.0, 0.0); // Move forward 10 meteres
    waitSeconds(WAIT_MED);
    pilot->setHeading(RIGHT_180_DEGREES);
    waitSeconds(WAIT_MED);
    pilot->moveRelativeMetresRestricted(10.0, 0.0, 0.0); // Move right 5 metres
    waitSeconds(WAIT_MED);
    pilot->setHeading(LEFT_180_DEGREES);
    waitSeconds(WAIT_MED);
}

void missionLookVertical(int droneId)
{
    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();

    camera->setTiltPan(17.0f, 0); // Look up
    waitSeconds(3);
    camera->setTiltPan(-45.0f, 0); // Look down
    waitSeconds(3);
    camera->setForward();
    waitSeconds(3);
}

void missionLookHorizontal(int droneId)
{
    ControlPtr       control = g_drones[droneId]->getDroneController();
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();
    VideoDriverPtr   video   = g_drones[droneId]->getVideoDriver();

    camera->setTiltPan(0.0f, 45.0f); // Look right
    waitSeconds(3);
    camera->setTiltPan(0.0f, -45.0f); // Look left
    waitSeconds(3);
    camera->setForward();
    waitSeconds(3);
}





