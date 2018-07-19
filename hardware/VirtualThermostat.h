#pragma once

#include <map>
#include "ThermostatHardware.h"

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


class VirtualThermostat : public CThermostatHardware
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

	//return the thermostat mode 
	virtual std::string GetCurrentMode(std::string &devIdx);
	//return the thermostat room temperature 
	virtual std::string GetRoomTemperature(std::string &devIdx);
	//set the thermostat mode 
	virtual bool SetThermostatState(const std::string &deviceIdx, const int newState);
	//convert interger state to string state : 0--> OFF 1-->ECO


  bool StartHardware();
  bool StopHardware();
  void Do_Work();

  std::shared_ptr<std::thread> m_thread;
  volatile bool m_stoprequested;
  int m_ScheduleLastMinute;


};

void UpdateVirtualThermostatOption(const uint64_t uidstr, float Power, float RoomTemp, float TempIdx, float SwitchIdx, float EcoTemp, float CoefProp, float ConforTemp, float CoefInteg, std::string &OnCmd, std::string &OffCmd);
std::string VirtualThermostatGetOption (const std::string optionName , const std::string &options );

extern VirtualThermostat * m_VirtualThermostat;
