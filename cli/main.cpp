#include "process.h"

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

	unsigned int uiThreadID = 0;
	HANDLE hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, NULL, CREATE_SUSPENDED, &uiThreadID);
	HANDLE hProcessThread = (HANDLE)_beginthreadex(NULL, 0, ProcessThread, NULL, CREATE_SUSPENDED, &uiThreadID);
	HANDLE hCommandThread = (HANDLE)_beginthreadex(NULL, 0, CommandThread, NULL, CREATE_SUSPENDED, &uiThreadID);

	//
	g_Log.Write(L"system: start.");

	if( false == Net.Start() ) 
		return;

	ResumeThread(hUpdateThread);
	ResumeThread(hProcessThread);
	ResumeThread(hCommandThread);

	//WCHAR wcsDomain[1024 + 1] = L"127.0.0.1";
	//WORD wPort = 65010;
	//if( false == Net.Connect(wcsDomain, wPort) )
	//	return;

	//
	DWORD &dwRunning = Net.m_dwRunning;

	while( 1 == InterlockedExchange((LONG*)&dwRunning, dwRunning) )
	{
		Sleep(500);
	}

	//
	g_Log.Write(L"system: end.");

	//
	return;
}
