#pragma once
#include "../_common/DefaultConfig.h"

class CConfig
	: public CIniFile
{
public:
	WCHAR wcsHost[1024+1] = {0,};
	WORD wPort = 0;

public:
	CConfig() {};
	virtual ~CConfig() {};

	bool LoadConfig();
};

extern CConfig g_Config;