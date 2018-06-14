#pragma once

#include <map>
#include "DomoticzHardware.h"

typedef std::map<int,  float  > T_Map_LastRoomTemp ;

//duratoiopn of integration window in min
#define INTEGRAL_DURATION 10

// hors gel temperature celcius
#define TEMPERATURE_HG   8
// of temperature celcius
#define TEMPERATURE_OFF  0

enum VirtualThermostatMode {
	Eco=0,
	Confor,
  FrostProtection,
  Off,
  EndMode
};

  typedef std::map<int,  CircularBuffer* > T_Map_CircularBuffer;


class VirtualThermostat : public CDomoticzHardwareBase
{
public:


	VirtualThermostat(const int ID);
	~VirtualThermostat();
	bool WriteToHardware(const char *pdata, const unsigned char length);

	void	ScheduleThermostat(int Minute);
	int		getPrevThermostatProg ( const char * devID , char * CurrentTime , std::string &Time );
	int		getNextThermostatProg ( const char * devID , char * CurrentTime , std::string &Time );
public:
	int		ThermostatGetEcoConfort (const char * devID , int CurrentTargetTemp , char * Duration);
	void	ThermostatToggleEcoConfort (const char * devID , char * setTemp , char * Duration);
	short	ComputeThermostatOutput ( int Min , int PowerPercent );
	int		ComputeThermostatPower ( int index ,float RoomTemp , float TargetTemp , float ExteriorTemp , float CoefProportional , float CoefIntegral , float CoefDerivated );
	int		GetConfortTempFromTimers  (const char * devID );
	int		GetEcoTempFromTimers  (const char * devID );
	std::string GetMode ( float curTemp , float EcoTemp, float ConfTemp );

	float GetEcoTemp (const char * devID );
	float GetConfortTemp (const char * devID );



	T_Map_LastRoomTemp Map_LastRoomTemp ;
	T_Map_CircularBuffer DeltaTemps;

	//thermostat function
	//return the thermostat available mode in string "OFF;ECO;CONFOR;AUTO;"
	virtual std::string GetAvailableMode();
	//return the thermostat mode 
	virtual std::string GetCurrentMode(TSqlRowQuery * row);
	//return the thermostat room temperature 
	virtual std::string GetRoomTemperature(TSqlRowQuery * row);
	//return the thermostat setpoint 
	virtual std::string GetSetPoint(TSqlRowQuery * row);
	//set the thermostat mode 
	virtual bool SetThermostatState(const std::string &deviceIdx, const int newState);
	//convert interger state to string state : 0--> OFF 1-->ECO
	virtual std::string ThermostatModeIntToString(int newState);
	//convert string state to int state : OFF-->0  ECO-->1
	virtual int ThermostatModeStringToInt(std::string &state);


  bool StartHardware();
  bool StopHardware();
  void Do_Work();

  boost::shared_ptr<boost::thread> m_thread;
  volatile bool m_stoprequested;
  int m_ScheduleLastMinute;


};

extern VirtualThermostat * m_VirtualThermostat;
