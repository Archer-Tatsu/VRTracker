#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <iomanip>
#include <string>
#include "aGlass.h"

using namespace std;

AGLASS_POINT aGlass::last_gaze{ 0.5,0.5 };
int aGlass::g_valid = 0; // 0:Invalid 1:Valid 2:Fetched and hasn't been updated

void AGLASS_CALL get_gaze_callback(const AGLASS_GAZE_DATA * gaze_data)
{
	if (gaze_data->valid == 1)
	{
		aGlass::last_gaze.x = gaze_data->gazePoint.x;
		aGlass::last_gaze.y = gaze_data->gazePoint.y;
		aGlass::g_valid = 1;
	}
	else
	{
		if (aGlass::g_valid == 2 || aGlass::g_valid == 0)
		{
			aGlass::last_gaze.x = gaze_data->gazePoint.x;
			aGlass::last_gaze.y = gaze_data->gazePoint.y;
			aGlass::g_valid = 0;
		}
	}
}

aGlass::aGlass()
{
	WORD wVersionRequerted = MAKEWORD(2, 2);
	WSADATA WSAData;
	WSAStartup(wVersionRequerted, &WSAData);
}


aGlass::~aGlass()
{
}

int aGlass::init()
{
	m_hSocketSend = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	m_hSocketRecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in addrRemtoe;
	addrRemtoe.sin_family = AF_INET;
	addrRemtoe.sin_port = htons(2000);
	addrRemtoe.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	sockaddr_in addrLocal;
	addrLocal.sin_family = AF_INET;
	addrLocal.sin_port = htons(2001);
	addrLocal.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	::bind(m_hSocketRecv, (const sockaddr*)&addrLocal, sizeof(sockaddr_in));

	sockaddr_in addrFrom;
	int iAddrLen = sizeof(sockaddr_in);
	int iRecvByte = 0;

	TIMEVAL tv = { 0, 0 };
	FD_SET fd = { 1, m_hSocketRecv };

	uint8_t buf[1024] = { "request" };

	int nByte = ::sendto(m_hSocketSend, (const char*)buf, sizeof(buf), 0, (struct sockaddr*)&addrRemtoe, sizeof(addrRemtoe));
	//std::cout << "send bytes: " << nByte << std::endl;

	if (0 != select(0, &fd, NULL, NULL, NULL))
	{
		memset(buf, 0, sizeof(buf));
		nByte = ::recvfrom(m_hSocketRecv, (char*)buf, sizeof(buf), 0,
			(struct sockaddr*)&addrFrom, &iAddrLen);
		//std::cout << "recv bytes: " << nByte << std::endl;
	}

	closesocket(m_hSocketSend);
	closesocket(m_hSocketRecv);


	char szTag[20] = { 0 };
	memcpy_s(szTag, sizeof(szTag), buf, sizeof(szTag));

	//if (strcmp(szTag, "invalid") == 0)
	//{
	//	cout << "invalid Coe, not found moudule." << endl;
	//	return 0;
	//}
	//else if (strcmp(szTag, "defaultLeft") == 0)
	//{
	//	cout << "use left eye default coe." << endl;
	//}
	//else if (strcmp(szTag, "defaultRight") == 0)
	//{
	//	cout << "use right eye default coe." << endl;
	//}
	//else if (strcmp(szTag, "response") == 0)
	//{
	//	cout << "use normal coe." << endl;
	//}
	//else
	//{
	//	cout << "unknow." << endl;
	//	return 0;
	//}

	const wchar_t* cntPath = L"C:\\Users\\jnlic\\AppData\\Local\\aGlassRuntime";

	init_param = { 0 };
	memcpy_s(init_param.calibCoe.coe, sizeof(init_param.calibCoe.coe), buf, sizeof(buf));
	init_param.pathSize = 512;
	init_param.path = new wchar_t[init_param.pathSize];
	memset(init_param.path, 0, init_param.pathSize * sizeof(wchar_t));

	wcscpy_s(init_param.path, init_param.pathSize, cntPath);
	memset(&init_param.path[wcslen(init_param.path)], 0, sizeof(wchar_t) * init_param.pathSize - wcslen(init_param.path) * sizeof(wchar_t));


	if (aGlass_init(&init_param))
		if (aGlass_start())
			return 1;
	//cout << "aGlass_init: " << aGlass_init(&init_param) << endl;
	//int mode = -1;
	//aGlass_get_mode(&mode);
	//cout << "aGlass Mode:" << mode << endl;
	//cout << "aGlass_start: " << aGlass_start() << endl;
	
	return 0;
}

int aGlass::startgaze()
{
	return aGlass_start_tracking(get_gaze_callback, get_gaze_callback);
}

int aGlass::stopgaze()
{
	return aGlass_stop_tracking();
}

int aGlass::finish()
{
	if (aGlass_stop() == AGLASS_SUCCESS) {
		if (aGlass_release() == AGLASS_SUCCESS) {
			return 1;
		}
	}
	return 0;
}
