/*
 * VideoCallback.cpp
 *
 *  Created on: Feb 8, 2019
 *      Author: slascos
 */
#include <memory>
#include <iostream>
#include <unistd.h>

#include "wscDrone.h"
#include "VideoFrame.h"


using namespace std;
using namespace wscDrone;

// Global variables
extern vector<shared_ptr<Bebop2>>     g_drones;
extern vector<unique_ptr<mutex>>      g_bufferGuards;
extern vector<shared_ptr<VideoFrame>> g_frames;

namespace wscDrone {

// onFrameRecieved
eARCONTROLLER_ERROR onFrameReceivedGeneric(ARCONTROLLER_Frame_t *frame, void *customData, int droneId)
{
    g_drones[droneId]->getVideoDriver()->Decode(frame);
    lock_guard<mutex> lock(*g_bufferGuards[droneId]);

    memcpy(g_frames[droneId]->getRawPointer(),
        g_drones[droneId]->getVideoDriver()->GetFrameRGBRawCstPtr(),
        g_frames[droneId]->getFrameSizeBytes());

    return ARCONTROLLER_OK;
}

eARCONTROLLER_ERROR onFrameReceived0(ARCONTROLLER_Frame_t *frame, void *customData)
{
    return onFrameReceivedGeneric(frame, customData, 0);
}
eARCONTROLLER_ERROR onFrameReceived1(ARCONTROLLER_Frame_t *frame, void *customData)
{
    return onFrameReceivedGeneric(frame, customData, 1);
}
eARCONTROLLER_ERROR onFrameReceived2(ARCONTROLLER_Frame_t *frame, void *customData)
{
    return onFrameReceivedGeneric(frame, customData, 2);
}

} // wscDrone
