#include "stdafx.h"
#include "EnOcean.h"
#include "../main/Logger.h"
#include "../main/Helper.h"
#include "../main/RFXtrx.h"
#include "../main/SQLHelper.h"

#include <string>
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
unsigned int CEnOcean::GetAdress(int unitid) {
	return(m_id_base + unitid);
}

uint64_t CEnOcean::CreateDevice(const int HardwareID, const char* ID, const int  unit, const unsigned char devType, const unsigned char subType, const unsigned char signallevel, const unsigned char batterylevel, const int nValue, const char* sValue, std::string &devname)
{
	uint64_t ulID = 0;
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT ID,Name, Used, SwitchType, nValue, sValue, LastUpdate, Options FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)", HardwareID, ID, unit, devType, subType);
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

//return the unit that correspond to the offset from controler base adress
int CEnOcean::getUnitFromDeviceId(unsigned int devIDx )
{
	return getUnitFromDeviceId(DeviceIDToString(devIDx) );
}
int CEnOcean::getUnitFromDeviceId(std::string devIDx  )
{
	std::vector<std::vector<std::string> > result;

	ToSensorsId(devIDx);

	//get Dev UnitCode 

	result = m_sql.safe_query("SELECT Address FROM EnoceanSensors WHERE (DeviceID='%s')  ", devIDx.c_str() );
	if (result.size() > 0) {
	return  atoi(result[0][0].c_str());
	}
	else
	return 0 ;
}


//return the sender adress fro enOcean deviceId 0x12345678
unsigned int CEnOcean::getSenderAdressFromDeviceId(std::string devIDx)
{
	int unitId = getUnitFromDeviceId(devIDx);
	unsigned int SenderAdress = GetAdress(unitId);
	return SenderAdress;
}

unsigned int CEnOcean::getSenderAdressFromDeviceId(unsigned int devIDx)
{
	return getSenderAdressFromDeviceId(DeviceIDToString(devIDx));
}
//DeviceId : ID of device in EnoceanSensor
//offsetID : offset of device from controler base adress 0..127
void CEnOcean::UpdateBaseAddress(std::string DeviceId , int offsetID ) {
	m_sql.safe_query("UPDATE EnoceanSensors SET %s=%d   WHERE (DeviceID = '%s' )", "Address" , offsetID , DeviceId.c_str() );

}

//find a base adress for enOcean device DeviceId : 0x12345678 
int CEnOcean::UpdateDeviceAddress(std::string DeviceId) {
	std::vector<std::vector<std::string> > result;
	bool  UsedUnitId[MAX_BASE_ADDRESS + 1];

	memset(UsedUnitId, 0, sizeof(UsedUnitId));
	//convert to 8 char
	ToSensorsId(DeviceId);

	//search if a same device ID already allocated exist
	result = m_sql.safe_query("SELECT Address FROM EnoceanSensors WHERE (DeviceId='%s') and (HardwareId=%d)  ", DeviceId.c_str(), m_HwdID);
	for (unsigned int i = 0; i < result.size(); i++)
	{
		//take the same
		int unitId = atoi(result[i][0].c_str());
		if (unitId != 0) {
			//UpdateBaseAddress(DeviceId, unitId );
			return unitId ;
		}
	}

	result = m_sql.safe_query("SELECT Address FROM EnoceanSensors WHERE (HardwareId=%d)  ",  m_HwdID );
	//get all BaseId allready affected to switch device
	for (unsigned int i = 0; i < result.size(); i++)
	{
		int unitId = atoi(result[i][0].c_str());
		unitId = unitId % MAX_BASE_ADDRESS ;//robustess
		//ID already used
		UsedUnitId[unitId] = true;
	}
	//find not used addess
	for (int i = 1; i < MAX_BASE_ADDRESS; i++)
	{
		if (UsedUnitId[i] == false) {
			UpdateBaseAddress(DeviceId, i);
			return i ;
		}
	}
	//device full 
	return 0;

}

int CEnOcean::UpdateDeviceAddress(unsigned int  DeviceId) 
	{
		return (UpdateDeviceAddress(DeviceIDToString(DeviceId)) );
	}
//test if deviceId exist in in database
//return 0 if not found
int CEnOcean::DeviceExist( unsigned int Deviceid)
{
return DeviceExist((char *)DeviceIDToString(Deviceid).c_str());
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
void CEnOcean::CreateSensors(char * szDeviceID, int rorg , int manufacturer, int profile, int ttype)
{
	m_sql.safe_query("INSERT INTO EnoceanSensors (HardwareID, DeviceID, Manufacturer, Rorg,Profile, [Type]) VALUES (%d,'%q',%d,%d,%d,%d)", m_HwdID, szDeviceID, manufacturer, rorg,profile, ttype);

}
void CEnOcean::CreateSensors(unsigned int DeviceID, int rorg, int manufacturer, int profile, int ttype)
{
	CreateSensors((char *)DeviceIDToString(DeviceID).c_str(), rorg, manufacturer, profile, ttype);
}

void CEnOcean::AddSensors(unsigned int DeviceID, int manufacturer, int profile, int ttype)
{

	if (!DeviceExist(DeviceID))
	{
		// If not found, add it to the database
		CreateSensors(DeviceID, 0,manufacturer, profile, ttype);
		_log.Log(LOG_NORM, "EnOcean: Sender_ID 0x%08X inserted in the database", DeviceID);
	}
	else
		_log.Log(LOG_NORM, "EnOcean: Sender_ID 0x%08X already in the database", DeviceID);
}

void CEnOcean::AddSensors(unsigned int DeviceID, int manufacturer, int profile, int ttype , int OffsetAddr )
{
	AddSensors(DeviceID, manufacturer, profile, ttype);

	UpdateBaseAddress(DeviceIDToString(DeviceID).c_str(),  OffsetAddr );
}

std::string CEnOcean::DeviceIDToString(unsigned int DeviceID)
{
	char szDeviceID[20];
	DeviceIDIntToChar(DeviceID, szDeviceID);
	return szDeviceID;
}

void CEnOcean::ToSensorsId(std::string &DeviceId)
{
	while (DeviceId.size() < 8)
		DeviceId = '0' + DeviceId;
}

//convert device ID id from  buffer[] to unsigned int
unsigned int setArrayToInt(unsigned char m_buffer[])
{
	unsigned int id = (m_buffer[0] << 24) + (m_buffer[1] << 16) + (m_buffer[2] << 8) + m_buffer[3];
	return id;
}

//convert device ID id from   unsigned int to buffer[]  
void  setIntToArray(unsigned int sID, unsigned char buf[])
{

buf[0] = (sID >> 24) & 0xff;
buf[1] = (sID >> 16) & 0xff;
buf[2] = (sID >> 8) & 0xff;
buf[3] = sID & 0xff;
}

void DeviceIDIntToChar(unsigned int DeviceID ,  char szDeviceID[])
{
	sprintf(szDeviceID, "%08X", (unsigned int)DeviceID);

}
//convert divice ID string to long
unsigned int DeviceIdCharToInt(std::string &DeviceID) {
	unsigned int ID;
	std::stringstream s_strid;
	s_strid << std::hex << DeviceID;
	s_strid >> ID;
	return ID;
}

bool CEnOcean::getProfile(std::string szDeviceID, int &Manufacturer, int &Rorg, int &Profile, int &Type)
{
	Rorg = Profile = Type = Manufacturer = 0;

	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT  Rorg, Profile, [Type], Manufacturer FROM EnoceanSensors WHERE (HardwareID==%d) AND (DeviceID=='%q')", m_HwdID, szDeviceID.c_str());
	if (result.size() == 1)
	{
		// hardware device was already teached-in
		Rorg         = atoi(result[0][0].c_str());
		Profile      = atoi(result[0][1].c_str());
		Type         = atoi(result[0][2].c_str());
		Manufacturer = atoi(result[0][3].c_str());
		return true;
	}
	return false;
}
bool CEnOcean::getProfile(unsigned int DeviceID, int &Manufacturer, int &Rorg, int &Profile, int &Type)
{
	return getProfile(DeviceIDToString(DeviceID), Manufacturer, Rorg, Profile, Type);
}
