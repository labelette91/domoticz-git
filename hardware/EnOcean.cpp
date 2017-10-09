#include "stdafx.h"
#include "EnOcean.h"
#include "../main/Logger.h"
#include "../main/Helper.h"
#include "../main/RFXtrx.h"
#include "../main/SQLHelper.h"

#include <string>
#define BASEID_FIELD_NAME "Power"
CEnOcean::CEnOcean() {
	m_id_base = 0;
};
//return true if base adress reading
bool CEnOcean::IsRunning()
{
	return (m_id_base != 0);
}
//id offset of device
//src : source rocker
unsigned long CEnOcean::GetAdress(int unitid) {
	return(m_id_base + unitid);
}

uint64_t CEnOcean::CreateDevice(const int HardwareID, const char* ID, const int  unit, const unsigned char devType, const unsigned char subType, const unsigned char signallevel, const unsigned char batterylevel, const int nValue, const char* sValue, std::string &devname)
{
	uint64_t ulID = 0;
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT ID,Name, Used, SwitchType, nValue, sValue, LastUpdate, Options, Log FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)", HardwareID, ID, unit, devType, subType);
	if (result.size() == 0)
	{
		//Insert

		if (!m_sql.m_bAcceptNewHardware)
		{
			devname = "Ignored";
			return -1; //We do not allow new devices
		}

		if (devname != "")
			devname = "Unknown" + std::string(ID);
		m_sql.safe_query(
			"INSERT INTO DeviceStatus (HardwareID, DeviceID, Unit, Type, SubType, SignalLevel, BatteryLevel, nValue, sValue,Name) "
			"VALUES ('%d','%q','%d','%d','%d','%d','%d','%d','%q','%q')",
			HardwareID,
			ID, unit, devType, subType,
			signallevel, batterylevel,
			nValue, sValue, devname.c_str());

		//Get new ID
		result = m_sql.safe_query(
			"SELECT ID FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)",
			HardwareID, ID, unit, devType, subType);
		if (result.size() == 0)
		{
			_log.Log(LOG_ERROR, "Serious database error, problem getting ID from DeviceStatus!");
			return -1;
		}
		std::stringstream s_str(result[0][0]);
		s_str >> ulID;
	}
	return ulID;
}

//return the ID of DiviceId 
//0 if not exist 
long CEnOcean::GetDeviceId(std::string DeviceID, int HardwareId)
{

	long lid = 0;
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT ID  FROM DeviceStatus WHERE (DeviceID='%s') and (HardwareId=%d)  ", DeviceID.c_str(), HardwareId);
	if (result.size() > 0)
	{
		lid = atoi(result[0][0].c_str());
	}
	return lid;
}

//convert divice ID string to long
unsigned long DeviceIdToLong(std::string &DeviceID) {
	unsigned long ID;
	std::stringstream s_strid;
	s_strid << std::hex << DeviceID;
	s_strid >> ID;
	return ID;
}


int CEnOcean::getUnitFromDeviceId(unsigned long devIDx , int UnitCode )
{
	std::vector<std::vector<std::string> > result;

	//get Dev UnitCode 

	result = m_sql.safe_query("SELECT " BASEID_FIELD_NAME "   FROM DeviceStatus WHERE (DeviceID='%0X') and (Unit=%d) ", devIDx, UnitCode);
	if (result.size() > 0) {
	return  atoi(result[0][0].c_str());
	}
	else
	return 0 ;
}

//idx : ID of device in devistatus
//offsetID : offset of device from controler base adress 0..127
void CEnOcean::UpdateBaseAddress(std::string idx, int offsetID ) {
	m_sql.UpdateDeviceValue(BASEID_FIELD_NAME, offsetID , idx);
}

//find a base adress for enOcean device and store it in db
void CEnOcean::UpdateDeviceAddress(std::string idx) {
	std::vector<std::vector<std::string> > result;
	bool  UsedUnitId[MAX_BASE_ADDRESS + 1];
	int DevunitCode = 0;
	std::string DeviceId;

	memset(UsedUnitId, 0, sizeof(UsedUnitId));


	//get Dev UnitCode 
	result = m_sql.safe_query("SELECT Unit  , DeviceId  FROM DeviceStatus WHERE (ID=%s)  ", idx.c_str());
	if (result.size() > 0) {
		DevunitCode = atoi(result[0][0].c_str());
		DeviceId = result[0][1];
	}
	else
		return;

	//search if a same device ID already allocated exist
	result = m_sql.safe_query("SELECT " BASEID_FIELD_NAME " FROM DeviceStatus WHERE (DeviceId='%s') and (HardwareId=%d)  ", DeviceId.c_str(), m_HwdID);
	for (unsigned int i = 0; i < result.size(); i++)
	{
		//take the same
		int unitId = atoi(result[i][0].c_str());
		if (unitId != 0) {
			UpdateBaseAddress( idx, unitId );
			return;
		}

	}

	result = m_sql.safe_query("SELECT " BASEID_FIELD_NAME ", DeviceId FROM DeviceStatus WHERE (Type=%d) and (SubType=%d) and (HardwareId=%d)  ", pTypeLighting2, sTypeAC , m_HwdID );
	//get all BaseId allready affected to switch device
	for (unsigned int i = 0; i < result.size(); i++)
	{
		int unitId = atoi(result[i][0].c_str());
		std::string DeviceId = result[i][1];
		unsigned long ID = DeviceIdToLong(DeviceId);
		//if dummy virtual rocker swicth device 
		if ((ID>=m_id_base) && (ID< m_id_base + MAX_BASE_ADDRESS )) {
			unitId = ID % MAX_BASE_ADDRESS ;
		}
		UsedUnitId[unitId] = true;
	}
	for (int i = 1; i < MAX_BASE_ADDRESS; i++)
	{
		if (UsedUnitId[i] == false) {
			UpdateBaseAddress(idx, i);
			return;
		}
	}


}

//test if deviceId exist in in database
//return 0 if not found
int CEnOcean::DeviceExist( unsigned int Deviceid)
{
char szDeviceID[20];

sprintf(szDeviceID, "%08X", (unsigned int)Deviceid);

return DeviceExist(szDeviceID);

}

int CEnOcean::DeviceExist(char * szDeviceID)
{

	std::vector<std::vector<std::string> > result;

	result = m_sql.safe_query("SELECT ID  FROM EnoceanSensors WHERE (DeviceID=='%q')", szDeviceID);
	//not found
	if (result.size() < 1)
		return 0;
	else
		return atoi(result[0][0].c_str());


}

//create sensor in database
void CEnOcean::CreateSensors(char * szDeviceID, int manufacturer, int profile, int ttype)
{
	m_sql.safe_query("INSERT INTO EnoceanSensors (HardwareID, DeviceID, Manufacturer, Profile, [Type]) VALUES (%d,'%q',%d,%d,%d)", m_HwdID, szDeviceID, manufacturer, profile, ttype);

}
void CEnOcean::CreateSensors(unsigned int DeviceID, int manufacturer, int profile, int ttype)
{
	char szDeviceID[20];

	sprintf(szDeviceID, "%08X", (unsigned int)DeviceID);

	CreateSensors(szDeviceID, manufacturer, profile, ttype);

}