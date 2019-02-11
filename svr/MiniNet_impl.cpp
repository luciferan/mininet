#include "../_network_2_/MiniNet.h"
#include "../_network_2_/Connector.h"
#include "../_common/Log.h"
#include "../_common/util.h"

//
eResultCode CMiniNet::DoUpdate(INT64 biCurrTime)
{
	if( 0 == m_biUpdateTime )
		m_biUpdateTime = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);

	//
	if( m_biUpdateTime < biCurrTime )
	{
		m_biUpdateTime = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);
		
		//
		wstring wstrReport = {};
		wstrReport.append(L"ConnectorState: ");
		wstrReport.append(CConnectorMgr::GetInstance().GetStateReport());

		g_PerformanceLog.Write(wstrReport.c_str());
	}

	return eResultCode::RESULT_SUCC;
}