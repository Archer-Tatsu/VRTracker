// BackgroundAppTest.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
//#include "BackgroundAppTest.h"
#include "GameMovementMonitor.h"
#include "aGlass.h"
#include "getopt.h"
#include <set>
#include <thread>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <io.h>

using namespace std;

static void usage(const char *exe) {//p:s:f:d:r:h:w
	fprintf(stderr,
		"%s [-p path] [-s subname] [-f filename] [-d duration] [-r framerate] [-h height_vd] [-w wait]\n"
		"\t-p ... Path to save data\n"
		"\t-s ... Subject's name\n"
		"\t-f ... Data file name (a.k.a video file name)\n"
		"\t-d ... Video duration (second, correct to millisecond)\n"
		"\t-r ... Video frame rate\n"
		"\t-h ... Default window's height of Virtual Desktop\n"
		"\t-w ... Time to wait before getting (second)\n",
		exe);
	return;
}

struct passdata {
	GameMovementMonitor &pgmm;
	aGlass &aglass;
	ostringstream &pdata;
};


//void CALLBACK DataRecord(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime) {
//	static int i = 0;
//	//passdata* alldata = (passdata*)nIDEvent;
//
//	//alldata->pdata << setiosflags(ios::fixed);
//	//alldata->pdata << dwTime << setw(15) << alldata->pgmm.last_pose.lat << setw(15) << alldata->pgmm.last_pose.lon << setw(15) << alldata->pgmm.last_pose.roll << endl;
//
//	cout << ++i << endl;
//	//if (++i == alldata->psample)
//	//{
//	//	int a = KillTimer(NULL, alldata->ptimer);
//	//	alldata->pflag = true;
//	//	alldata->pgmm.stopflag = true;
//	//}
//}

void runhmd(GameMovementMonitor* gmm, bool ini)
{
	if (ini)
	{
		gmm->MainLoop();
	}
}

void runaglass(aGlass* aglass, bool ini)
{
	if (ini)
	{
		aglass->startgaze();
	}
}

void datarecord(passdata &alldata, double time)
{
	alldata.pdata << setiosflags(ios::fixed);
	alldata.pdata << time << setw(15) << alldata.pgmm.last_pose.lat << setw(15) << alldata.pgmm.last_pose.lon << setw(15) << alldata.pgmm.last_pose.roll << \
		setw(15) << aGlass::last_gaze.x << setw(15) << aGlass::last_gaze.y << setw(15) << aGlass::g_valid << endl;
	//cout << setiosflags(ios::fixed);
	//cout << time << setw(15) << alldata.pgmm.last_pose.lat << setw(15) << alldata.pgmm.last_pose.lon << setw(15) << alldata.pgmm.last_pose.roll << \
		setw(15) << aGlass::last_gaze.x << setw(15) << aGlass::last_gaze.y << setw(15) << aGlass::g_valid << endl;
	aGlass::g_valid = 2;
}

// test.exe path subname filename duration framerate
void main(int argc, char** argv)
{

	string path;
	string subname;
	string filename;
	float duration = 0;
	float framerate = 0;
	int height_vd = 0;
	int wait = 0;
	
	// Read command line arguments
	char ch;
	
	while ((ch = getopt(argc, argv, "p:s:f:d:r:h:w:")) != -1)
	{
		switch (ch)
		{
		case('p'):
			path = optarg;
			break;
		case('s'):
			subname = optarg;
			break;
		case('f'):
			filename = optarg;
			break;
		case('d'):
			duration = strtof(optarg, 0);
			break;
		case('r'):
			framerate = strtof(optarg, 0);
			break;
		case('h'):
			height_vd = strtol(optarg, 0, 0);
			break;
		case('w'):
			wait = strtol(optarg, 0, 0);
			break;
		default:
			return usage(argv[0]);
		}
	}
	if (argc < 15)
		return usage(argv[0]);

	GameMovementMonitor gmm;
	aGlass aglass;

	bool gmm_ini = gmm.Init();
	bool aglass_ini = aglass.init();

	LARGE_INTEGER frequence;
	QueryPerformanceFrequency(&frequence);

	ostringstream data;

	passdata alldata{ gmm,aglass,data };
	

	float samplerate = 0.5 / framerate;//2 sample per frame time
	int sampletotal = static_cast<int>(duration * 2 * framerate);

	LARGE_INTEGER set_rate;
	set_rate.QuadPart = (LONGLONG)(frequence.QuadPart * samplerate);

	LARGE_INTEGER initial, previous, current;
	int i;

	HWND hwnd = NULL;

	RECT rect;

	//Get VD window
	while (hwnd == NULL)
		hwnd = FindWindow(NULL, "Virtual Desktop");
	std::cout << "Window found." << endl;
	
	while (1)
	{
		GetWindowRect(hwnd, &rect);
		LONG height = rect.bottom - rect.top;
		if (height<= height_vd)
		{
			//SendMessage(hwnd, WM_KEYDOWN, VK_SPACE, NULL);
			//for (i = 0; i < 100; i++);
			//SendMessage(hwnd, WM_KEYUP, VK_SPACE, NULL);

			SetForegroundWindow(hwnd);
			INPUT input[2];
			input[0].ki.wVk = VK_SPACE;
			input[0].type = INPUT_KEYBOARD;
			input[1].ki.wVk = VK_SPACE;
			input[1].type = INPUT_KEYBOARD;
			input[1].ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(2, input, sizeof(INPUT));

			std::cout << "Video Loaded. Pause." << endl;
			break;
		}
	}

	// Wait for video buffering
	QueryPerformanceCounter(&initial);
	previous = initial; current = initial;
	for (i = 0; i < static_cast<int>(wait * 2 * framerate); i++)
	{
		while (current.QuadPart - previous.QuadPart < set_rate.QuadPart)
			QueryPerformanceCounter(&current);

		//double temp = ((double)(current.QuadPart - previous.QuadPart)) * 1000.0 / frequence.QuadPart;
		previous = current;
	}
	std::cout << "Listening trigger event..." << endl;

	thread t1(runhmd, &gmm, gmm_ini);
	t1.detach();

	thread t2(runaglass, &aglass, aglass_ini);
	t2.detach();

	// Wait for trigger pressing
	while (1)
	{
		if (gmm.GetTrigger())
		{
			gmm.startflag = 1;
			break;
		}
	}

	// Resume video playback
	SendMessage(hwnd, WM_KEYDOWN, VK_SPACE, NULL);
	SendMessage(hwnd, WM_KEYUP, VK_SPACE, NULL);

	std::cout << "start tracking" << endl;
	QueryPerformanceCounter(&initial);
	previous = initial; current = initial;
	for (i = 0; i < sampletotal; i++)
	{
		while (current.QuadPart - previous.QuadPart < set_rate.QuadPart)
			QueryPerformanceCounter(&current);

		double temp = ((double)(current.QuadPart - previous.QuadPart)) * 1000.0 / frequence.QuadPart;
		previous = current;

		thread t3(datarecord, alldata, temp);
		t3.detach();
	}
	std::cout << "end tracking" << endl;
	//UINT_PTR timer = SetTimer(NULL, 1, 1, DataRecord);
	
	//passdata alldata{ gmm,data,timer,sampletotal,stopflag };
	gmm.stopflag = true;
	aglass.stopgaze();
	aglass.finish();

	

	string fullpath = path + "\\" + subname;
	if (_access(fullpath.c_str(), 0) != 0) {
		string cmd = "md " + fullpath;
		system(cmd.c_str());
	}
	string filepath = fullpath + "\\" + filename + ".txt";

	ofstream textout(filepath);
	textout << data.str();
	textout << ((double)(current.QuadPart - initial.QuadPart)) / frequence.QuadPart << endl;
	textout.close();
	return;

}