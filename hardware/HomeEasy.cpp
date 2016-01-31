
#include "stdafx.h"
#include "HomeEasy.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef __arm__
	#include <linux/i2c-dev.h>
	#include <linux/i2c.h> 
	#include <unistd.h>
	#include <sys/ioctl.h>
  #include <sched.h>    
	#include <wiringPi.h>
#endif
#include <math.h> 
#include "../main/Helper.h"
#include "../main/Logger.h"
#include "hardwaretypes.h"
#include "../main/RFXtrx.h"
#include "../main/localtime_r.h"
#include "../main/mainworker.h"

#include "HomeEasyTransmitter.h"

HomeEasy::HomeEasy(const int ID)
{
	m_stoprequested=false;
	m_HwdID=ID;

	int TXPIN = 0;
	HomeEasyRfTx = 0;

#ifdef __arm__
	if (wiringPiSetup() == -1)
	{
		_log.Log(LOG_ERROR, "failed to initialize wiring pi");
	}
	HomeEasyRfTx = new HomeEasyTransmitter(TXPIN, 0);
	HomeEasyRfTx->initPin();
#endif
}

HomeEasy::~HomeEasy()
{
}

bool HomeEasy::StartHardware()
{
#ifndef __arm__
//	return false;
#endif
	m_stoprequested=false;


	//Start worker thread
	m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&HomeEasy::Do_Work, this)));
	sOnConnected(this);
	m_bIsStarted=true;
	return (m_thread!=NULL);
}

bool HomeEasy::StopHardware()
{
	m_stoprequested=true;
	if (m_thread!=NULL)
		m_thread->join();
	m_bIsStarted=false;
	return true;
}

bool HomeEasy::WriteToHardware(const char *pdata, const unsigned char length)
{
	unsigned char devType=pdata[1];
	unsigned char subType=pdata[2];
	const tRBUF *pResponse=(const tRBUF *)pdata;

	if (devType==pTypeLighting2)
	{
		int unit=pResponse->LIGHTING2.unitcode;
		long id = pResponse->LIGHTING2.id1 << 24 | pResponse->LIGHTING2.id2 << 16 | pResponse->LIGHTING2.id3 << 8 | pResponse->LIGHTING2.id4;
		int cmd = pResponse->LIGHTING2.cmnd;
		_log.Log(LOG_TRACE, "HERF: Send Home Easy Id :%08X Unit:%d Cmd:%d" ,id , unit, cmd );
		if (HomeEasyRfTx){
#ifdef __arm__
			HomeEasyRfTx->setSwitch(cmd, id, unit);   
#endif
			}
		else
			_log.Log(LOG_ERROR, "HomeEasyRfTx not initialized");

	}
    
    return true;
}

void HomeEasy::Do_Work()
{
	int sec_counter = 0;

	_log.Log(LOG_STATUS, "HomeEasy: Worker started...");
	while (!m_stoprequested)
	{
		sleep_milliseconds(1000);
		if (m_stoprequested)
			break;
		sec_counter++;

		if (sec_counter % 12 == 0) {
			m_LastHeartbeat = mytime(NULL);
		}
	}
	_log.Log(LOG_STATUS,"HomeEasy: Worker stopped...");
}

#ifdef __arm__
void scheduler_realtime() {

	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if (sched_setscheduler(0, SCHED_RR, &p) == -1) {
		_log.Log(LOG_ERROR, "Failed to switch to realtime scheduler.");
	}
}

void scheduler_standard() {

	struct sched_param p;
	p.__sched_priority = 0;
	if (sched_setscheduler(0, SCHED_OTHER, &p) == -1) {
		_log.Log(LOG_ERROR, "Failed to switch to normal scheduler.");
	}
}

#endif

