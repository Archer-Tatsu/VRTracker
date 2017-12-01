#define _USE_MATH_DEFINES
#include "GameMovementMonitor.h"
#include <cmath>
#include <Windows.h>

GameMovementMonitor::GameMovementMonitor()
: m_pHMD(NULL)
, m_iTrackedControllerCount(0)
, m_iTrackedControllerCount_Last(-1)
, m_iValidPoseCount(0)
, m_iValidPoseCount_Last(-1)
, m_strPoseClasses("")
{
memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));
}

GameMovementMonitor::~GameMovementMonitor() {}

//void GameMovementMonitor::Run()
//{
//if (Init())
//{
//	MainLoop();
//	//textout << start_time << " " << now_time << endl;
//	//textout.close();
//}
//
//Shutdown();
//}

/////////////////////////////////////

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 GameMovementMonitor::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
Matrix4 matrixObj(
	matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
	matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
	matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
	matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
);
return matrixObj;
}

/////////////////////////////////////
bool GameMovementMonitor::Init()
{
	//Start this as a Background application
	vr::EVRInitError eVRInitError;
	m_pHMD = vr::VR_Init(&eVRInitError, vr::VRApplication_Background);
	if (!vr::VRSystem() || eVRInitError != vr::VRInitError_None)
		return false;

	//textout.open(textname);

	startflag = 0;
	stopflag = 0;

	//GetLocalTime(&sys_time);
	//start_time = sys_time.wSecond * 1000 + sys_time.wMilliseconds;
	//if (start_time > 49999)
	//	start_time = start_time - 60000;
	//now_time = start_time;

	return true;
}

vr::HmdQuaternion_t GameMovementMonitor::GetRotation(vr::HmdMatrix34_t matrix) {
	vr::HmdQuaternion_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}


/////////////////////////////////////
void GameMovementMonitor::PrintControllerInfo()
{
	vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd;
	if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
		return;
	if (!m_rTrackedDevicePose[unTrackedDevice].bPoseIsValid)
		return;

	//const Matrix4 & mat = m_rmat4DevicePose[unTrackedDevice];

	//Vector4 temp = mat * Vector4(1, 0, 0, 0);

	Vector3 temp;
	temp.x = atan2(2.0*(q.w*q.z + q.x*q.y), 1.0 - 2.0*(pow(q.z, 2) + pow(q.x, 2)));
	temp.y = asin(2.0*(q.w*q.x - q.y*q.z));
	temp.z = -atan2(2.0*(q.w*q.y + q.x*q.z), 1.0 - 2.0*(pow(q.y, 2) + pow(q.x, 2)));

	//cout << setiosflags(ios::fixed);
	//cout << setw(15) << temp.y * 180 / M_PI << setw(15) << temp.z * 180 / M_PI << endl;

	//textout << setiosflags(ios::fixed);
	//textout << setw(15) << temp.y * 180 / M_PI << setw(15) << temp.z * 180 / M_PI << endl;

	last_pose.roll = temp.x * 180 / M_PI;
	last_pose.lat = temp.y * 180 / M_PI;
	last_pose.lon = temp.z * 180 / M_PI;

	/*
	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
	if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
	continue;

	if (m_pHMD->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
	continue;

	//m_iTrackedControllerCount += 1;

	if (!m_rTrackedDevicePose[unTrackedDevice].bPoseIsValid)
	continue;

	const Matrix4 & mat = m_rmat4DevicePose[unTrackedDevice];

	Vector4 center = mat * Vector4(0, 0, 0, 1);
	//probably need to get the tracked volume size here too
	cout << "X:" << center.x << " Y:" << center.y << " Z" << center.z << endl;
	}
	*/
}

/////////////////////////////////////
void GameMovementMonitor::UpdateHMDMatrixPose()
{
	if (!m_pHMD)
		return;
	vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseSeated, 0, m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount);
	//vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";

	int nDevice = vr::k_unTrackedDeviceIndex_Hmd;
	if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		q = GetRotation(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);


		
	//m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
	/*
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
	if (1)//m_rTrackedDevicePose[nDevice].bPoseIsValid)
	{
	m_iValidPoseCount++;
	m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
	if (m_rDevClassChar[nDevice] == 0)
	{
	switch (m_pHMD->GetTrackedDeviceClass(nDevice))
	{
	case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
	case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
	case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
	case vr::TrackedDeviceClass_Other:             m_rDevClassChar[nDevice] = 'O'; break;
	case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
	default:                                       m_rDevClassChar[nDevice] = '?'; break;
	}
	}
	m_strPoseClasses += m_rDevClassChar[nDevice];
	}
	}

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
	m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].invert();
	}
	*/
}


/////////////////////////////////////
void GameMovementMonitor::MainLoop()
{
	do
	{
		//MSG msg;
		//int itemp = PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE);
		//if (msg.message == WM_TIMER)
		//	break;

		//std::this_thread::sleep_for(k_MonitorInterval);

		//vr::VREvent_t Event;
		//while (vr::VRSystem()->PollNextEvent(&Event, sizeof(Event)))
		//{
		//	switch (Event.eventType)
		//	{
		//	case vr::VREvent_Quit:
		//		exit(0);
		//		// NOTREAHED

		//	case vr::VREvent_TrackedDeviceActivated:
		//		cout << "Device" << Event.trackedDeviceIndex << "attached." << endl;
		//		break;
		//	case vr::VREvent_TrackedDeviceDeactivated:
		//		cout << "Device" << Event.trackedDeviceIndex << "detached." << endl;
		//		break;
		//	case vr::VREvent_TrackedDeviceUpdated:
		//		cout << "Device" << Event.trackedDeviceIndex << "Updated." << endl;
		//		break;
		//	}
		//}
		if (startflag)
		{
			UpdateHMDMatrixPose();

			//Grab the controller info and print it out
			if (m_pHMD)
			{
				PrintControllerInfo();
			}
		}
		else
		{
			/*vr::TrackedDeviceIndex_t nRightHand = 0;
			nRightHand = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
			m_pHMD->GetControllerState(nRightHand, &state, sizeof(state));
			uint64_t mask = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
			uint64_t trigger = state.ulButtonPressed & mask;
			if (trigger > 0)
				startflag = 1;*/
		}
		//UpdateHMDMatrixPose();

		//GetLocalTime(&sys_time);
		//now_time = sys_time.wSecond * 1000 + sys_time.wMilliseconds;
		//if (start_time < 0 && now_time > 49999)
		//	now_time = now_time - 60000;

		//cout << now_time << endl;
	} while (!stopflag); // (-1 != itemp);

	Shutdown();
}

/////////////////////////////////////
void GameMovementMonitor::Shutdown()
{
	vr::VR_Shutdown();
	//textout.close();
}

bool GameMovementMonitor::GetTrigger()
{
	vr::TrackedDeviceIndex_t nRightHand = 0;
	nRightHand = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
	m_pHMD->GetControllerState(nRightHand, &state, sizeof(state));
	uint64_t mask = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
	uint64_t trigger = state.ulButtonPressed & mask;
	if (trigger > 0)
		return true;
	else
		return false;
}