#pragma once
#include <aGlass_vr_api.h>
#include <WinSock2.h>

class aGlass
{
	AGLASS_INIT_PARAM init_param;
	
	SOCKET m_hSocketSend;
	SOCKET m_hSocketRecv;

public:


	aGlass();
	~aGlass();

	static AGLASS_POINT last_gaze;
	static int g_valid; // 0:Invalid 1:Valid 2:Fetched and hasn't been updated

	int init();
	int startgaze();
	int stopgaze();
	int finish();
};

