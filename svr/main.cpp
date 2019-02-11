#include "Process.h"

//
void main()
{
	CExceptionReport::GetInstance().ExceptionHandlerBegin();

	//
	LoadConfig();

	//
	CMiniNet &Net = CMiniNet::GetInstance();
	if( false == Net.Initialize() )
		return;

	//
	DWORD &dwRunning = Net.m_dwRunning;
	unsigned int uiThreadID = 0;
	HANDLE hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, NULL, CREATE_SUSPENDED, &uiThreadID);
	HANDLE hProcessThread = (HANDLE)_beginthreadex(NULL, 0, ProcessThread, NULL, CREATE_SUSPENDED, &uiThreadID);
	//HANDLE hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, NULL, CREATE_SUSPENDED, &uiThreadID);

	//
	g_Log.Write(L"system: start.");

	if( false == Net.ListenStart(g_Config.wcsHost, g_Config.wPort) )
		return;

	ResumeThread(hUpdateThread);
	ResumeThread(hProcessThread);
	//ResumeThread(hMonitorThread);

	//
	char cmd[128+1] = {0,};
	while( 1 == InterlockedExchange((LONG*)&dwRunning, dwRunning) )
	{
		gets_s(cmd);

		if( 0 == strcmp(cmd, "/exit") )
			break;

		if( 0 == strcmp(cmd, "/restart") )
		{
			Net.AcceptRestart();
		}
		if( 0 == strcmp(cmd, "/astop") )
			Net.AcceptStop();
		if( 0 == strcmp(cmd, "/astart") )
			Net.AcceptStart();

		//Sleep(500);
	}

	//
	Net.Stop();
	WaitForSingleObject(hUpdateThread, INFINITE);
	CRecvPacketQueue::GetInstance().ForceActivateQueueEvent();
	WaitForSingleObject(hProcessThread, INFINITE);
	//WaitForSingleObject(hMonitorThread, INFINITE);

	g_Log.Write(L"system: end.");

	//
	return;
}