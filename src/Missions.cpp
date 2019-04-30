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


void startDrone(int droneId)
{
    g_drones[droneId]->getDroneController()->start();
    cout << "Done CONTROL START" << endl;
    waitSeconds(1);
    g_drones[droneId]->getVideoDriver()->start();
    cout << "Done VIDEO START" << endl;
    g_drones[droneId]->getCameraControl()->setForward();
    waitSeconds(1);
    cout << "Done drone start" << endl;
}

void takeoffDrone(int droneId) {
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


// Look around with the camera, then move forward 1 metre, than back 1 metre.
//void mission1(shared_ptr<Bebop2> bebop2Ptr)
void mission1(int droneId)
{
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();
    PilotPtr         pilot   = g_drones[droneId]->getPilot();

    camera->setTiltPan(17.0f, 45.0f); // Look up and to the right
    waitSeconds(5);
    camera->setTiltPan(-17.0f, -45.0f); // Look down and to the left
    waitSeconds(5);
    camera->setForward();
    waitSeconds(5);

    pilot->moveRelativeMetres(2.0, 0.0); // Move forward 1 metre
    pilot->moveRelativeMetres(-2.0, 0.0); // move backward 1 meter
}

// Fly a triangle by strafing to the right, then turning 120 degrees, 3x.
void missionTriange(int droneId)
{
    PilotPtr pilot = g_drones[droneId]->getPilot();

    const float STEP_DISTANCE = 2.0f;
    const float TURN_ANGLE =  -120.0f;

    pilot->moveRelativeMetres(0.0, STEP_DISTANCE); // Move right 5 metres
    pilot->setHeading(TURN_ANGLE);                      // rotate counter-clockwise 90
    pilot->moveRelativeMetres(0.0, STEP_DISTANCE); // Move left 10 metre
    pilot->setHeading(TURN_ANGLE);                      // rotate counter-clockwise 90
    pilot->moveRelativeMetres(0.0, STEP_DISTANCE); // Move left 10 metre
    pilot->setHeading(TURN_ANGLE);                      // rotate counter-clockwise 90
}


void missionLookVertical(int droneId)
{
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();

    camera->setTiltPan(17.0f, 0); // Look up
    waitSeconds(3);
    camera->setTiltPan(-45.0f, 0); // Look down
    waitSeconds(3);
    camera->setForward();
    waitSeconds(3);
}

void missionLookHorizontal(int droneId)
{
    CameraControlPtr camera  = g_drones[droneId]->getCameraControl();

    camera->setTiltPan(0.0f, 45.0f); // Look right
    waitSeconds(3);
    camera->setTiltPan(0.0f, -45.0f); // Look left
    waitSeconds(3);
    camera->setForward();
    waitSeconds(3);
}





