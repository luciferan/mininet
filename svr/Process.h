#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

//
#include "../_network_2_/MiniNet.h"
#include "../_network_2_/PacketDataQueue.h"
#include "../_common/ExceptionReport.h"
#include "../_common/util.h"

#include "../_common/Log.h"
#include "./Config.h"

//
bool LoadConfig();

unsigned int WINAPI UpdateThread(void *p);
unsigned int WINAPI ProcessThread(void *p);

unsigned int WINAPI MonitorThread(void *p);

//
#endif //__PROCESS_H__
