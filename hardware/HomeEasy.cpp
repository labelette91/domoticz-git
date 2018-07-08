
#include "stdafx.h"

//#define  WITH_GPIO

#include "HomeEasy.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "RfTxRx/Print.h"

#ifdef WITH_GPIO

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sched.h>    
#include <wiringPi.h>

//#include "RCSwitch.h"
#include "RfTxRx/RcOok.h"
#include "RfTxRx/Sensor.h"

#include "RfTxRx/DecodeOTIO.h"
#include "RfTxRx/HagerDecoder.h"

//rfm69 
#define SS 0
#define RF69_IRQ_PIN 0 
#define RF69_IRQ_NUM 0 
#include "RfTxRx/RFM69.h"
#include "RfTxRx/RFM69registers.h"

#endif
#include "RfTxRx/SPI.h"
#include "../main/Logger.h"
#include "../main/localtime_r.h"
#include "../main/RFXtrx.h"

#include "RfTxRx/HomeEasyTransmitter.h"
#include "../main/SQLHelper.h"
#include "RfTxRx/hager.h"

#include "RfTxRx/RcOok.h"
#include "RfTxRx/DecodeHomeEasy.h"

#include "RfTxRx/Record.h"


SPIClass SPI;

//typedef map<int, Sensor > T_Map_Sensor;
//T_Map_Sensor Sensors ;

int  SetGpioInterruptMode(int   bcmGpioPin, int mode);

int HomeEasy::RXPIN;

char OokReceivedCode[128];

#ifdef WITH_GPIO

OregonDecoderV2 orscV2;

DecodeHomeEasy HEasy(1);

DecodeOTIO     Otio(5);

HagerDecoder   hager;

#endif

#include "RfTxRx/fifo.cpp"

//TFifo Fifo;
TRecord Record;


HomeEasy::HomeEasy(const int ID)
{
	m_stoprequested=false;
	m_HwdID=ID;

	TXPIN = 0;
	RXPIN = 0;
	int SPI_CHAN = 1;
	int Spi_speed = 500000;
	HomeEasyRfTx = 0;

//	Fifo.Clear();
	Record.init();

	//input pin in mode1
	//output pin in mode2

	TSqlQueryResult result = m_sql.safe_query("SELECT Mode1, Mode2, Mode3, Mode4, Mode5, Mode6 FROM Hardware WHERE (ID='%d')", ID );
	if (result.size() > 0)
	{
		RXPIN = atoi(result[0][0].c_str());
		TXPIN = atoi(result[0][1].c_str());
		_log.Debug(DEBUG_NORM, "HERF: RXPin:%d TXPin:%d",  RXPIN, TXPIN);

	}
#ifdef WITH_GPIO
	if (wiringPiSetup() == -1)
	{
		_log.Log(LOG_ERROR, "failed to initialize wiring pi");
	}

	HomeEasyRfTx = new HomeEasyTransmitter(TXPIN, 0);
	//HomeEasyRfTx->initPin();

  //set hager TX led pin
  HagerSetPin(TXPIN, 0);

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
	_log.Debug(DEBUG_NORM, "HERF: RFM69 initialized ", TXPIN, RXPIN);

	if (TXPIN != RXPIN)
		radio->setMode(RF69_MODE_SLEEP);
	else
		radio->setMode(RF69_MODE_RX);

	// if receiver defined
	if (RXPIN >= 0)
		wiringPiISR(RXPIN, INT_EDGE_BOTH, &handleInterrupt);
	else
	{
		/* disable interupt */
		//SetGpioInterruptMode(RXPIN, 0);
	}
#endif
}

HomeEasy::~HomeEasy()
{
}

bool HomeEasy::StartHardware()
{
#ifndef WITH_GPIO
//	return false;
#endif
	m_stoprequested=false;


	//Start worker thread
	m_thread = std::shared_ptr<std::thread>(new std::thread(std::bind(&HomeEasy::Do_Work, this)));
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

std::string CmdStr[] = {

"CMD_ECO    ", 
"CMD_CONFOR ", 
"CMD_HGEL   ", 
"CMD_ARRET  ", 
"CMD_CONFIG ", 

};

#ifdef WITH_GPIO
void hagerSends(byte id4, byte cmnd)
{
  _log.Debug(DEBUG_NORM, "HERF: Send HAGER  Id :%08X Cmd:%s Cmd:%d", id4 , CmdStr[cmnd].c_str() , cmnd );
  HagerSends(id4, cmnd);

}

//unit code = 0 : configuration
//unit code = 1 : cmnd = 0 eco   1: confor
//unit code = 2 : cmnd = 0 hgel  1: confor
//unit code = 3 : cmnd = 0 arret 1: confor
void ManageHager( byte unitcode, byte id4 , byte cmnd )
{
	//unit code = 0 : configuration
	if (unitcode==16)
    hagerSends(id4, CMD_CONFIG);
	//unit code = 1 : cmnd = 0 eco   1: confor
	else if(unitcode==1)
    hagerSends(id4,cmnd);
	//unit code = 2 : cmnd = 0 hgel  1: confor
	else if(unitcode==2)
	{
		if (cmnd==0)
      hagerSends(id4,CMD_HGEL);
		else
			//confor
      hagerSends(id4,cmnd);
	}	
	//unit code = 3 : cmnd = 0 arret 1: confor
	else if(unitcode==3)
	{
		if (cmnd==0)
      hagerSends(id4,CMD_ARRET);
		else
			//confor
      hagerSends(id4,cmnd);
	}		
}
#endif


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
    byte id4 = pResponse->LIGHTING2.id4;

    if (HomeEasyRfTx) {
#ifdef WITH_GPIO
      HomeEasyRfTx->initPin();
      radio->setMode(RF69_MODE_TX);
      //attente une secone max pour emetre si emission en cours -80--> -70
      radio->WaitCanSend(-70);
      //send

      if (subType == sTypeHEU) {
        HomeEasyRfTx->setSwitch(cmd != 0, id, unit);
        _log.Debug(DEBUG_NORM, "HERF: Send Home Easy Id :%08X Unit:%d Cmd:%d", id, unit, cmd);
      }
      else 		if (subType == sTypeAC)
      {
        ManageHager(unit, id4, cmd);
      }
      HomeEasyRfTx->deactivatePin();

      //if same rx/tx pin : goto receive state after transmit
      if (TXPIN != RXPIN)
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

void DumpHex (byte * data , byte len , char * mesage )
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
  int p ;
  int pinData;
	memset(OokReceivedCode, 0, sizeof(OokReceivedCode));
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

#ifdef WITH_GPIO
		//if tace is enable and dump pulse data is enable
		if ( (!Record.empty()) && (_log.isTraceEnabled()) &&(!_log.TestFilter("RCDATA")) )
					_log.Debug(DEBUG_NORM, "RCDATA %s", Record.ToString(1).c_str());

		//while a pulse has been recorded
		while (!Record.empty())
		{
			p = Record.get();

		/* pinData == 0 if  p<0 */
        //pinData : input signal value before interrupt
			if (p < 0)			{
				pinData = 0;
				p = -p;
			}
			else pinData = 1;

        /*low to high transition : low duration*/
	      if (pinData == 0) p += 100;else p -= 100;

        if (orscV2.nextPulse(p))
        {
          orscV2.sprint(OregonSensorV2::_sensorId, OokReceivedCode);
          orscV2.resetDecoder();
        }
        if (HEasy.nextPulse(p, pinData))
        {
          orscV2.sprint(OregonSensorV2::_sensorId, HEasy.getData(), HEasy.getBytes(), OokReceivedCode);
          HEasy.resetDecoder();
        }

        if (Otio.nextPulse(p, pinData))
        {
          orscV2.sprint(OregonSensorV2::_sensorId, Otio.getData(), Otio.getBytes(), OokReceivedCode);
          Otio.resetDecoder();
        }

        /*    if (hager.nextPulse(p))
        {
        int nb = buildHagerDataMessage(Data ) ;
        orscV2.sprint(OregonSensorV2::_sensorId, Data, nb, RCSwitch::OokReceivedCode);
        hager.resetDecoder();
        }
        */
        /* if message received */
        if (OokReceivedCode[0])
        {
          _log.Debug(DEBUG_NORM, "RCOOK %s", OokReceivedCode );

          Sensor *s = Sensor::getRightSensor((char*)OokReceivedCode );
          if (s != NULL)
          {
            //if correct decoded sensor and different value since last receive
            if (s->isDecoded())
            {
              //find sensor
              Sensor * sensor = FindSensor(s->getSensID());
              if (sensor == 0) {
                //            Sensors[s->getSensID()] = sensor;
                _log.Debug(DEBUG_NORM, "SENSOR: new sensor added %s ID:%08X", s->getSensorName().c_str(), s->getSensID());
              }

              //if sensor value as changed
              if ((sensor == 0) || (*sensor != *s))
              {
                //update current sensor value
                Sensors[s->getSensID()] = s;
                if (s->available(Sensor::haveTemperature))
                {
                  SendTempHumSensor(s->getSensID(), !s->isBatteryLow(), (float)s->getTemperature(), (int)s->getHumidity(), "Home TempHum", 12,sTypeTH1);
                  _log.Debug(DEBUG_NORM, "RCOOK %s ID Code:%04X  Rolling:%0X Temp : %f Humidity : %f Channel : %d ", s->getSensorName().c_str(), s->getSensType(), s->getSensID(), s->getTemperature(), s->getHumidity(), s->getChannel());
                }
                if (s->available(Sensor::haveOnOff))
                {
                  SendSwitch(s->getSensID(), s->getChannel(), 0xff, s->getOnOff() != 0, 0, "HomeEasy");
                  _log.Debug(DEBUG_NORM, "RCOOK %s ID Code:%04X  Rolling:%08X OnOff : %d Unit : %d ", s->getSensorName().c_str(), s->getSensType(), s->getSensID(), s->getOnOff(), s->getChannel());
                }
                if (s->available(Sensor::havePower))
                {
                  SendKwhMeter(s->getSensType(), s->getChannel(), 0xff, s->getPower(), s->getTotalPower() / 1000.0 / 223.666, "POWER");
                  _log.Debug(DEBUG_NORM, "RCOOK %s Code:%04X  Power : %d Total : %d ", s->getSensorName().c_str(), s->getSensType(), s->getPower(), s->getTotalPower());
                }
                //delete old value
                if (sensor != 0) delete sensor;
              }
              else
                delete s;
            }
            else
            {
              delete s;
              _log.Debug(DEBUG_NORM, "RCOOK Sensor Id :%04X checksum error", s->getSensType());
            }
          }
          else
            _log.Debug(DEBUG_NORM, "RCOOK Sensor unknown");

        }
        OokReceivedCode[0] = 0 ;
		}
#endif



	}
	_log.Log(LOG_STATUS,"HomeEasy: Worker stopped...");
}

void scheduler_realtime() {

#ifdef WITH_GPIO
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if (sched_setscheduler(0, SCHED_RR, &p) == -1) {
		_log.Log(LOG_ERROR, "Failed to switch to realtime scheduler.");
	}
#endif
}

void scheduler_standard() {
#ifdef WITH_GPIO
	struct sched_param p;
	p.__sched_priority = 0;
	if (sched_setscheduler(0, SCHED_OTHER, &p) == -1) {
		_log.Log(LOG_ERROR, "Failed to switch to normal scheduler.");
	}
#endif
}


#ifdef WITH_GPIO
int wiringPiMode = WPI_MODE_PINS;
int  SetGpioInterruptMode( int   pin , int mode )
{

	const char *modeS;
	char  pinS[8];
	pid_t pid;
	int   bcmGpioPin;

	int   model, rev, mem, maker, overVolted;

	piBoardId(&model, &rev, &mem, &maker, &overVolted);
	_log.Debug(DEBUG_NORM, "PI Board  model:%d   rev:%d mem:%d maker:%d overVolted:%d", model, rev, mem, maker, overVolted);

	if (model == PI_MODEL_CM)
		wiringPiMode = WPI_MODE_GPIO;
	else
		wiringPiMode = WPI_MODE_PINS;



	if ((pin < 0) || (pin > 63))
		return wiringPiFailure(WPI_FATAL, "wiringPiISR: pin must be 0-63 (%d)\n", pin);

	/**/ if (wiringPiMode == WPI_MODE_UNINITIALISED)
		return wiringPiFailure(WPI_FATAL, "wiringPiISR: wiringPi has not been initialised. Unable to continue.\n");
	else if (wiringPiMode == WPI_MODE_PINS)
		bcmGpioPin = wpiPinToGpio(pin);
	else if (wiringPiMode == WPI_MODE_PHYS)
		bcmGpioPin = physPinToGpio(pin);
	else
		bcmGpioPin = pin;

	// Now export the pin and set the right edge
	//	We're going to use the gpio program to do this, so it assumes
	//	a full installation of wiringPi. It's a bit 'clunky', but it
	//	is a way that will work when we're running in "Sys" mode, as
	//	a non-root user. (without sudo)


if (mode == INT_EDGE_SETUP)
	modeS = "none";
else if (mode == INT_EDGE_FALLING)
	modeS = "falling";
else if (mode == INT_EDGE_RISING)
	modeS = "rising";
else
	modeS = "both";

sprintf(pinS, "%d", bcmGpioPin);

if ((pid = fork()) < 0)	// Fail
	return wiringPiFailure(WPI_FATAL, "wiringPiISR: fork failed: %s\n", strerror(errno));

if (pid == 0)	// Child, exec
{
	if (access("/usr/local/bin/gpio", X_OK) == 0)
	{
		execl("/usr/local/bin/gpio", "gpio", "edge", pinS, modeS, (char *)NULL);
		return wiringPiFailure(WPI_FATAL, "wiringPiISR: execl failed: %s\n", strerror(errno));
	}
	else if (access("/usr/bin/gpio", X_OK) == 0)
	{
		execl("/usr/bin/gpio", "gpio", "edge", pinS, modeS, (char *)NULL);
		return wiringPiFailure(WPI_FATAL, "wiringPiISR: execl failed: %s\n", strerror(errno));
	}
	else
		return wiringPiFailure(WPI_FATAL, "wiringPiISR: Can't find gpio program\n");
}
else		// Parent, wait
	wait(NULL);
}

void HomeEasy::handleInterrupt() {

	static unsigned int duration;
	static unsigned long lastTime;

	long time = micros();
	duration = time - lastTime;
	lastTime = time;
	word p = (unsigned short int) duration;

	byte pinData = digitalRead(HomeEasy::RXPIN);
	if (p != 0) {
		if (pinData == 0)
			Record.put(-p);
		else
			Record.put(p);
	}
}
#endif

