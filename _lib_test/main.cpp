#include "../_common/util.h"

int main()
{
	CTimeSet currtime;
	currtime._test();

	CLog log;

	for( int idx = 0; idx < 5; ++idx )
		log.write(L"%04d - %s", idx, L"a");

	return 0;
}