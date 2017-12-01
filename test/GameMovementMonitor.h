#pragma once

#include "Matrices.h"
#include <openvr.h>

using namespace std;

class GameMovementMonitor
{
private:
	vr::IVRSystem *m_pHMD;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_mat4HMDPose;

	vr::HmdQuaternion_t q;

	vr::VRControllerState_t state;

	//ofstream textout;
	//get message temp flag
			  //SYSTEMTIME sys_time;
			  //int start_time;
			  //int now_time;

public:
	//string textname;

	GameMovementMonitor();

	~GameMovementMonitor();

	bool Init();
	void MainLoop();

	bool startflag;
	bool stopflag;

	struct posture {
		float lat;
		float lon;
		float roll;
	} last_pose;

	bool GetTrigger();

protected:

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

	vr::HmdQuaternion_t GetRotation(vr::HmdMatrix34_t matrix);
	
	void PrintControllerInfo();
	void UpdateHMDMatrixPose();
	
	void Shutdown();

};