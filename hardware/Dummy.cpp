#include "stdafx.h"
#include "Dummy.h"
#include "../main/Helper.h"
#include "../main/Logger.h"
#include "../main/SQLHelper.h"
#include "../main/mainworker.h"
#include "../main/WebServer.h"
#include "../webserver/cWebem.h"
#include "../json/json.h"
#include "hardwaretypes.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sstream>
#include "../main/localtime_r.h"
#include "VirtualThermostat.h"

CDummy::CDummy(const int ID)
{
	m_HwdID=ID;
	m_bSkipReceiveCheck = true;
}

CDummy::~CDummy(void)
{
	m_bIsStarted=false;
}

void CDummy::Init()
{
}

bool CDummy::StartHardware()
{
	Init();
	m_stoprequested = false;


	//Start worker thread
	m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CDummy::Do_Work, this)));
	sOnConnected(this);
	m_bIsStarted = true;
	return (m_thread != NULL);
}

bool CDummy::StopHardware()
{
	m_stoprequested = true;
	if (m_thread != NULL)
		m_thread->join();
	m_bIsStarted = false;
	return true;
}
void CDummy::Do_Work()
{
	int sec_counter = 0;
	_log.Log(LOG_STATUS, "CDummy: Worker started...");
	while (!m_stoprequested)
	{
		sleep_milliseconds(1000);
		if (m_stoprequested)
			break;
		sec_counter++;

		if (sec_counter % 12 == 0) {
			m_LastHeartbeat = mytime(NULL);
		}
		time_t atime = mytime(NULL);
		struct tm ltime;
		localtime_r(&atime, &ltime);
		if (ltime.tm_min != m_ScheduleLastMinute)
		{
				m_VirtualThermostat.ScheduleThermostat(ltime.tm_min);
				m_ScheduleLastMinute = ltime.tm_min;
		}

	}
	_log.Log(LOG_STATUS, "CDummy: Worker stopped...");
}

bool CDummy::WriteToHardware(const char *pdata, const unsigned char length)
{
#ifdef _DEBUG
	if (length < 2)
		return false;
	std::string sdevicetype = RFX_Type_Desc(pdata[1], 1);
	if (pdata[1] == pTypeGeneral)
	{
		const _tGeneralDevice *pMeter = reinterpret_cast<const _tGeneralDevice*>(pdata);
		sdevicetype += "/" + std::string(RFX_Type_SubType_Desc(pMeter->type, pMeter->subtype));
	}
	_log.Log(LOG_STATUS, "Dummy: Received null operation for %s", sdevicetype.c_str());
#endif
	return true;
}

//Webserver helpers
namespace http {
	namespace server {
		void CWebServer::RType_CreateVirtualSensor(WebEmSession & session, const request& req, Json::Value &root)
		{
			if (session.rights != 2)
			{
				session.reply_status = reply::forbidden;
				return; //Only admin user allowed
			}

			std::string idx = request::findValue(&req, "idx");
			std::string ssensorname = request::findValue(&req, "sensorname");
			std::string ssensortype = request::findValue(&req, "sensortype");
			std::string soptions = request::findValue(&req, "sensoroptions");
			if ((idx == "") || (ssensortype.empty()) || (ssensorname.empty()))
				return;

			uint16_t fullType;
			std::stringstream ss;
			ss << std::hex << ssensortype;
			ss >> fullType;

			int SensorType = fullType >> 8;
			int SensorSubType = fullType & 0xFF;

			int HwdID = atoi(idx.c_str());

			//Make a unique number for ID
			std::vector<std::vector<std::string> > result;
			result = m_sql.safe_query("SELECT MAX(ID) FROM DeviceStatus");

			unsigned long nid = 0; //could be the first device ever

			if (result.size() > 0)
			{
				nid = atol(result[0][0].c_str()) + 1;
			}
			unsigned long vs_idx = nid; // OTO keep idx to be returned before masking
			nid += 82000;

			bool bPrevAcceptNewHardware = m_sql.m_bAcceptNewHardware;
			m_sql.m_bAcceptNewHardware = true;

			uint64_t DeviceRowIdx = m_sql.CreateDevice(HwdID, SensorType, SensorSubType, ssensorname, nid, soptions);

			m_sql.m_bAcceptNewHardware = bPrevAcceptNewHardware;

			if (DeviceRowIdx != -1)
			{
				root["status"] = "OK";
				root["title"] = "CreateVirtualSensor";
				std::stringstream ss;
				ss << vs_idx;
				root["idx"] = ss.str().c_str();
			}
		}
	}
}

