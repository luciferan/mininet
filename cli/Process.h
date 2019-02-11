#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

//
#include "../_network_2_/MiniNet.h"
#include "../_common/ExceptionReport.h"
#include "../_common/util.h"

#include "../_common/Log.h"

//
bool LoadConfig();

unsigned int WINAPI UpdateThread(void *p);
unsigned int WINAPI ProcessThread(void *p);
unsigned int WINAPI CommandThread(void *p);

//
#endif //__PROCESS_H__
