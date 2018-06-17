#pragma once

#include <iostream>
#include "DomoticzHardware.h"

class CThermostatHardware : public CDomoticzHardwareBase
{
public:

	//thermostat available mode in string "OFF,ECO,CONFOR,AUTO," separated by comma, 
	// and in the order from 0 to N and 0 = OFF 1 = ECO, 2 = CONFOR 3 = AUTO
	std::string  AvailableMode ;

	//thermostat function
	//return the thermostat available mode in string "OFF,ECO,CONFOR,AUTO," separated by comma , and 0=OFF 1=ECO, 2=CONFOR 3=AUTO
	virtual std::string GetAvailableMode() { return AvailableMode ; };
	virtual void        SetAvailableMode(std::string  pAvailableMode ) { AvailableMode = pAvailableMode; };
	//return the thermostat mode 
	virtual std::string GetCurrentMode( std::vector<std::string> * row) { return ""; };
	//return the thermostat room temperature 
	virtual std::string GetRoomTemperature(std::vector<std::string> * row) { return ""; };
	//return the thermostat current setpoint 
	virtual std::string GetSetPoint(std::vector<std::string> * row) { return ""; };
	//set the thermostat mode : newState = integer thermostat mode , in the order in AvailableMode
	virtual bool SetThermostatState(const std::string &deviceIdx, const int newState) { return true; };



};

