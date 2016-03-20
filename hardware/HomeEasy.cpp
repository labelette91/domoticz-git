
#include "stdafx.h"

//#define  __arm__

#include "HomeEasy.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef __arm__

#include <unistd.h>
#include <sys/ioctl.h>
#include <sched.h>    
#include <wiringPi.h>

#include "RCSwitch.h"
#include "RcOok.h"
#include "Sensor.h"

#include "SPI.h"

#endif
#include "../main/Logger.h"
#include "../main/localtime_r.h"
#include "../main/RFXtrx.h"

#include "HomeEasyTransmitter.h"
#include "../main/SQLHelper.h"

#ifdef __arm__
SPIClass SPI;
RCSwitch *rc ;

#endif

HomeEasy::HomeEasy(const int ID)
{
	m_stoprequested=false;
	m_HwdID=ID;

	TXPIN = 0;
	RXPIN = 0;
	int SPI_CHAN = 1;
	int Spi_speed = 500000;
	HomeEasyRfTx = 0;

	//input pin in mode1
	//output pin in mode2

	TSqlQueryResult result = m_sql.Query("SELECT Mode1, Mode2, Mode3, Mode4, Mode5, Mode6 FROM Hardware WHERE (ID='%d')", ID );
	if (result.size() > 0)
	{
		TXPIN = atoi(result[0][0].c_str());
		RXPIN = atoi(result[0][1].c_str());
		_log.Log(LOG_TRACE, "HERF: RXPin:%d TXPin:%d", TXPIN, RXPIN);

	}
	TXPIN = 5;
#ifdef __arm__
	if (wiringPiSetup() == -1)
	{
		_log.Log(LOG_ERROR, "failed to initialize wiring pi");
	}
	HomeEasyRfTx = new HomeEasyTransmitter(TXPIN, 0);
	//HomeEasyRfTx->initPin();

	//  spiSetup ( 1, 500000) ;
	if (SPI.Setup(SPI_CHAN, Spi_speed))
	{
		_log.Log(LOG_ERROR, "failed to open the SPI bus: ");
	}
	//if receiver declared , init receive pin
	if ( RXPIN != -1 )
		pinMode(RXPIN, INPUT);

	radio = new RFM69(0, 0);
	radio->initialize(RF69_433MHZ, 1, 100);
	_log.Log(LOG_TRACE, "HERF: RFM69 initialized ", TXPIN, RXPIN);

	if (TXPIN != RXPIN)
		radio->setMode(RF69_MODE_SLEEP);
	else
		radio->setMode(RF69_MODE_RX);

	// if receiver defined
	if (RXPIN >= 0)
		rc = new RCSwitch(RXPIN, -1);
	else
		rc = 0;

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
			HomeEasyRfTx->initPin();
			radio->setMode(RF69_MODE_TX);
			//attente une secone max pour emetre si emission en cours -80--> -70
			radio->WaitCanSend(-70);
			//send
			HomeEasyRfTx->setSwitch(cmd, id, unit);   
			//if same rx/tx pin : goto receive state after transmit
			if (TXPIN!= RXPIN)
				radio->setMode(RF69_MODE_SLEEP);
			else
				radio->setMode(RF69_MODE_RX);

#endif
			}
		else
			_log.Log(LOG_ERROR, "HomeEasyRfTx not initialized");

	}
    
    return true;
}
void DumpHex (char * data , byte len , char * mesage )
{

	for (byte i = 0; i < len; i++) {
		sprintf(mesage, "%02X", data[i]);
		mesage++;
		mesage++;

	}
	*mesage = 0;
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

#ifdef __arm__
		if (rc!=0)
//			if (rc->OokAvailable())
			if (!rc->Fifo.Empty())

			{
				char dataStr[100];
				char data[100];
				byte len;
				std::string message;

				//rc->getOokCode(message);
        //strncpy((char*) message,(const char*)rc->Fifo.Get(len),90 ) ;
				memcpy((char*)data, (const char*)rc->Fifo.Get(len), 90);
				printf("%x ", data[0]);

				printf("%d \n", len );

				DumpHex(data, len, dataStr);
				
				message = "OSV2 " + std::string(dataStr);
				_log.Log(LOG_TRACE, "OSV2 %s", message.c_str());

				Sensor *s = Sensor::getRightSensor((char*)message.c_str());
				if (s != NULL)
				{
					_log.Log(LOG_TRACE, "Temp : %f Humidity : %f Channel : %d ", s->getTemperature(), s->getHumidity(), s->getChannel());
				}
				delete s;
			}
#endif



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

