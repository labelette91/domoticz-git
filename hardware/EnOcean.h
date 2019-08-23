#ifndef CEnOcean_H
#define	CEnOcean_H

#include "ASyncSerial.h"
#include "DomoticzHardware.h"
#include "../main/WebServer.h"
#include "../json/json.h"

#define MAX_BASE_ADDRESS 128

#define uint unsigned int

typedef enum {
	RORG_RPS = 0xF6,
	RORG_1BS = 0xD5,
	RORG_4BS = 0xA5,
	RORG_VLD = 0xD2,
	RORG_MSC = 0xD1,
	RORG_ADT = 0xA6,
	RORG_SM_LRN_REQ = 0xC6,
	RORG_SM_LRN_ANS = 0xC7,
	RORG_SM_REC = 0xA7,
	RORG_SYS_EX = 0xC5,
	RORG_UTI = 0xD4
} ESP3_RORG;

//! Packet type (ESP3)
typedef enum
{
	PACKET_RESERVED = 0x00,	//! Reserved
	PACKET_RADIO = 0x01,	//! Radio telegram
	PACKET_RESPONSE = 0x02,	//! Response to any packet
	PACKET_RADIO_SUB_TEL = 0x03,	//! Radio sub telegram (EnOcean internal function )
	PACKET_EVENT = 0x04,	//! Event message
	PACKET_COMMON_COMMAND = 0x05,	//! Common command
	PACKET_SMART_ACK_COMMAND = 0x06,	//! Smart Ack command
	PACKET_REMOTE_MAN_COMMAND = 0x07,	//! Remote management command
	PACKET_PRODUCTION_COMMAND = 0x08,	//! Production command
	PACKET_RADIO_MESSAGE = 0x09,	//! Radio message (chained radio telegrams)
	PACKET_RADIO_ADVANCED = 0x0a  //! Advanced Protocol radio telegram

} PACKET_TYPE;


#define SIZE_LINK_TABLE 24 

typedef struct {
	int		Profile;
	uint	SenderId;
	int		Channel;
}T_LINK_TABLE;

typedef struct {
	unsigned int	Profile;
	int				Manufacturer;
	int				CurrentSize;
	int				MaxSize;
	T_LINK_TABLE	LinkTable[SIZE_LINK_TABLE];

}T_SENSOR;

typedef 	std::map<unsigned int, T_SENSOR > T_SENSOR_MAP;

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

	unsigned int GetOffsetAdress(int unitid);
	

	virtual void SendDimmerTeachIn(const char *pdata, const unsigned char length) = 0 ;

	uint64_t CEnOcean::CreateDevice(const int HardwareID, const char* ID, const int  unit, const unsigned char devType, const unsigned char subType, const unsigned char signallevel, const unsigned char batterylevel, const int nValue, const char* sValue, std::string &devname, int SwitchType, const char * SensorId );

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

	bool getProfile(std::string szDeviceID, int &Manufacturer, int &Rorg, int &Profile, int &Type);

	bool getProfile(unsigned int DeviceID , int &Manufacturer, int &Rorg, int &Profile, int &Type);

	void UpdateProfileSensors(char * szDeviceID, int rorg, int profile, int ttype);

	void TeachIn(std::string& idx);
	void GetNodeList(http::server::WebEmSession & session, const http::server::request& req, Json::Value &root);
	void SetCode(http::server::WebEmSession & session, const http::server::request& req, Json::Value &root);

	bool CheckIsGatewayAdress(unsigned int id);

private:
	virtual bool ParseData() { return true; };

	virtual bool sendFrame(unsigned char frametype, unsigned char *databuf, unsigned short datalen, unsigned char *optdata, unsigned char optdatalen) { return true; };
	virtual bool sendFrameQueue(unsigned char frametype, unsigned char *databuf, unsigned short datalen, unsigned char *optdata, unsigned char optdatalen) { return true; };

protected:
	void parse_PACKET_REMOTE_MAN_COMMAND(unsigned char m_buffer[], int m_DataSize, int m_OptionalDataSize);
	void sendVld(unsigned int sID, int channel, int value);
	void sendVld(unsigned int sID, unsigned char *data, int DataLen);

	void SendRpsTeachIn(unsigned int sID);
	void Send1BSTeachIn(unsigned int sID);
	void Send4BSTeachIn(unsigned int sID);
	void remoteLearning(unsigned int destID, bool StartLearning, int channel = 0);
	void setRorg(unsigned char * buff);
	void unlock(unsigned int destID, unsigned int code);
	void lock(unsigned int destID, unsigned int code);
	void setcode(unsigned int destID, unsigned int code);
	void ping(unsigned int destID);
	void action(unsigned int destID);
	void getProductId();
	void addSensorManuf(uint SensorId, uint Manuf);
	void addSensorProfile(uint SensorId, uint Profile);
	void getLinkTableMedadata(uint SensorId);
	void setLinkTableMedadata(uint SensorId, int csize, int maxsize);
	void getProductFunction(uint SensorId);

	void getallLinkTable(uint SensorId, int begin, int end);
	void addLinkTable(uint DeviceId, int entry, int profile, uint sensorId, int channel);
	void printSensors();

	unsigned long m_id_base;
	int m_Seq;
	T_SENSOR_MAP m_sensors;

};

//convert id from  buffer[] to unsigned int
unsigned int DeviceArrayToInt(unsigned char m_buffer[]);
void         DeviceIntToArray(unsigned int sID, unsigned char buf[]);

void         DeviceIDIntToChar(unsigned int DeviceID,  char szDeviceID[]);
std::string  DeviceIDIntToChar(unsigned int DeviceID);
unsigned int DeviceIdCharToInt(std::string &DeviceID);
std::string GetLighting2StringId(unsigned int id);

bool TypFnAToB(const char * st, unsigned char bin[], int  *trame_len);
const char* Get_EnoceanManufacturer(unsigned long ID);
const char* Get_Enocean4BSType(const int Org, const int Func, const int Type);

void ToSensorsId(std::string &DeviceId);


#endif

