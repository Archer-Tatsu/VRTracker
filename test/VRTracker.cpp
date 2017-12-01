// BackgroundAppTest.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
//#include "BackgroundAppTest.h"
#include "GameMovementMonitor.h"
#include "aGlass.h"
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
void main(int argc, char* argv[])
{

	string path(argv[1]);
	string subname(argv[2]);
	string filename(argv[3]);
	string time(argv[4]);
	string fps(argv[5]);

	GameMovementMonitor gmm;
	aGlass aglass;

	bool gmm_ini = gmm.Init();
	bool aglass_ini = aglass.init();

	LARGE_INTEGER frequence;
	QueryPerformanceFrequency(&frequence);

	ostringstream data;
	
	thread t1(runhmd, &gmm, gmm_ini);
	t1.detach();

	thread t2(runaglass, &aglass, aglass_ini);
	t2.detach();

	passdata alldata{ gmm,aglass,data };
	
	float duration = atof(time.c_str());
	float framerate = atof(fps.c_str());

	float samplerate = 0.5 / framerate;//2 sample per frame time
	int sampletotal = static_cast<int>(duration * 2 * framerate);

	LARGE_INTEGER set_rate;
	set_rate.QuadPart = (LONGLONG)(frequence.QuadPart * samplerate);

	LARGE_INTEGER initial, previous, current;
	int i;

	cout << "start traking" << endl;
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
	cout << "end tracking" << endl;
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