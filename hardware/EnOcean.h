#ifndef CEnOcean_H
#define	CEnOcean_H

#include "ASyncSerial.h"
#include "DomoticzHardware.h"

#define MAX_BASE_ADDRESS 128

class CEnOcean : public AsyncSerial, public CDomoticzHardwareBase
{
	friend class CEnOceanESP3;

public:
    /**
    * Opens a serial device.
    * \param devname serial device name, example "/dev/ttyS0" or "COM1"
    * \param baud_rate serial baud rate
    * \param opt_parity serial parity, default even
    * \param opt_csize serial character size, default 7bit
    * \param opt_flow serial flow control, default none
    * \param opt_stop serial stop bits, default 1
    * \throws boost::system::system_error if cannot open the
    * serial device
    */
//	CEnOcean(const int ID, const std::string& devname, const int type);

	CEnOcean();

	//return true if base adress reading
	bool IsRunning();
	//id offset of device
	//src : source rocker
	unsigned int GetAdress(int unitid);

	virtual void SendDimmerTeachIn(const char *pdata, const unsigned char length) = 0 ;

	uint64_t CreateDevice(const int HardwareID, const char* ID, const int unit, const unsigned char devType, const unsigned char subType, const unsigned char signallevel, const unsigned char batterylevel, const int nValue, const char* sValue, std::string &devname);
	
	int UpdateDeviceAddress(std::string idx );

	int UpdateDeviceAddress(unsigned int  DeviceId);

	void UpdateBaseAddress(std::string idx, int offsetID);

	static int getUnitFromDeviceId(unsigned int devIDx);

	static int getUnitFromDeviceId(std::string devIDx);

	unsigned int getSenderAdressFromDeviceId(unsigned int devIDx);

	unsigned int getSenderAdressFromDeviceId(std::string devIDx);

	int DeviceExist(unsigned int Deviceid);

	int DeviceExist(char * szDeviceID);

	void CreateSensors(char * szDeviceID, int rorg, int manufacturer, int profile, int ttype);

	void CreateSensors(unsigned int DeviceID, int rorg, int manufacturer, int profile, int ttype);

	void AddSensors(unsigned int DeviceID, int manufacturer, int profile, int ttype);

	void AddSensors(unsigned int DeviceID, int manufacturer, int profile, int ttype, int OffsetAddr);

	static std::string DeviceIDToString(unsigned int DeviceID);

	static void ToSensorsId(std::string &DeviceId);

	bool getProfile(std::string szDeviceID, int &Manufacturer, int &Rorg, int &Profile, int &Type);

	bool getProfile(unsigned int DeviceID , int &Manufacturer, int &Rorg, int &Profile, int &Type);

protected:

	unsigned long m_id_base;
};

//convert id from  buffer[] to unsigned int
unsigned int DeviceIDArrayToInt(unsigned char m_buffer[]);
void         DeviceIDIntToArray(unsigned int sID, unsigned char buf[]);

void         DeviceIDIntToChar(unsigned int DeviceID,  char szDeviceID[]);
unsigned int DeviceIdCharToInt(std::string &DeviceID);


#endif

