/*
 * Missions.h
 *
 *  Created on: Feb 7, 2019
 *      Author: slascos
 */

#ifndef SRC_MISSIONS_H_
#define SRC_MISSIONS_H_

void startDrone(int droneId);
void takeoffDrone(int droneId);
void landDrone(int droneId);
void stopDrone(int droneId);
void setFlightAltitude(int droneId, float heightMeters);

void mission1(int droneId);
void missionBox(int droneId);
void missionBoxInv(int droneId);
void missionTForward(int droneId);
void missionTTurn(int droneId);
void missionTurnBack(int droneId);

void missionTriange(int droneId);

void missionLookVertical(int droneId);
void missionLookHorizontal(int droneId);

void missionDance();
void missionDance2();

#endif /* SRC_MISSIONS_H_ */
