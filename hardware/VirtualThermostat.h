#pragma once

#include <map>

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


class VirtualThermostat
{
public:


	VirtualThermostat();
	~VirtualThermostat();

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
  char *          GetModeStr( VirtualThermostatMode mode );
  VirtualThermostatMode  GetModeInt( const char * mode );
  bool SetMode ( const std::string &idx,VirtualThermostatMode mode );
  bool SetMode ( const std::string &idx,const char * mode );
  std::string GetAvailableMode() ;
  bool SetThermostatState(const std::string &idx, const int newState);

};

extern VirtualThermostat m_VirtualThermostat;
