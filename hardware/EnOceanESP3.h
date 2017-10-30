#pragma once

#include <vector>
#include "ASyncSerial.h"
#include "DomoticzHardware.h"
#include "EnOcean.h"


#define ENOCEAN3_READ_BUFFER_SIZE 65*1024

#define uint unsigned int

#define SIZE_LINK_TABLE 24 

typedef struct {
	int		Profile;	
	uint	SenderId;
	int		Channel;
}T_LINK_TABLE;



typedef struct  {
	unsigned int	Profile;
	int				Manufacturer;
	int				CurrentSize;
	int				MaxSize;
	T_LINK_TABLE	LinkTable[SIZE_LINK_TABLE];

}T_SENSOR;

typedef 	std::map<unsigned int, T_SENSOR > T_SENSOR_MAP;

class CEnOceanESP3: public CEnOcean
{
	enum _eEnOcean_Receive_State
	{
		ERS_SYNCBYTE=0,
		ERS_HEADER,
		ERS_DATA,
		ERS_CHECKSUM
	};
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
	CEnOceanESP3(const int ID, const std::string& devname, const int type);

    ~CEnOceanESP3();
	bool WriteToHardware(const char *pdata, const unsigned char length);
	void SendDimmerTeachIn(const char *pdata, const unsigned char length);
	void TeachIn(std::string& idx, std::string& hardwareid);

private:
	void Init();
	bool StartHardware();
	bool StopHardware();
	bool OpenSerialDevice();
	void Do_Work();
	bool ParseData();
	void Add2SendQueue(const char* pData, const size_t length);
	float GetValueRange(const float InValue, const float ScaleMax, const float ScaleMin=0, const float RangeMax=255, const float RangeMin=0);

	bool sendFrame(unsigned char frametype, unsigned char *databuf, unsigned short datalen, unsigned char *optdata, unsigned char optdatalen);
	bool sendFrameQueue(unsigned char frametype, unsigned char *databuf, unsigned short datalen, unsigned char *optdata, unsigned char optdatalen);

	void ParseRadioDatagram();

	_eEnOcean_Receive_State m_receivestate;
	int m_wantedlength;

	boost::shared_ptr<boost::thread> m_thread;
	volatile bool m_stoprequested;
    int m_Type;
	std::string m_szSerialPort;

	bool m_bBaseIDRequested;

	// Create a circular buffer.
	unsigned char m_ReceivedPacketType;
	int				m_DataSize;
	int				m_OptionalDataSize;
    unsigned char m_buffer[ENOCEAN3_READ_BUFFER_SIZE];
	int m_bufferpos;
	int m_retrycntr;
	int m_Seq;
	boost::mutex m_sendMutex;
	std::vector<std::string> m_sendqueue;

	T_SENSOR_MAP m_sensors ;


	/**
     * Read callback, stores data in the buffer
     */
    void readCallback(const char *data, size_t len);
	void sendVld(unsigned int sID, int channel, int value);
	void SendRpsTeachIn(unsigned int sID);
	void Send1BSTeachIn(unsigned int sID);
	void Send4BSTeachIn(unsigned int sID);
	void TestData(char *  data);
	void TestData(char * sdata, char * optData);
	void remoteLearning(unsigned int destID, bool StartLearning,  int channel = 0 );
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
	void setLinkTableMedadata(uint SensorId, int csize,int maxsize);
	void getProductFunction(uint SensorId);

	void getallLinkTable(uint SensorId, int begin, int end);
	void addLinkTable(uint DeviceId, int entry , int profile , uint sensorId, int channel);
	void printSensors();






};

typedef enum	
{
 	UNLOCK                                = 0x001, //Unlock
	LOCK                                  = 0x002, //Lock
	SETCODE                               = 0x003, //Set code
	QUERYID                               = 0x004, //Query ID
	QUERYID_ANSWER                        = 0x604, //Query ID answer
	QUERYID_ANSWER_EXT                    = 0x704, //! Ext Query Id Answer
	ACTION                                = 0x005, //Action
	PING                                  = 0x006, //Ping
	PING_ANSWER                           = 0x606, //Ping answer
	QUERY_FUNCTION                        = 0x007, //Query function
	QUERY_FUNCTION_ANSWER                 = 0x607, //Query function answer
	QUERY_STATUS                          = 0x008, //Query status
	QUERY_STATUS_ANSWER                   = 0x608, //Query status answer
	REMOTE_LEARNIN                        = 0x201, //Remote learn in
	REMOTE_FLASH_WRITE                    = 0x203, //Remote flash write
	REMOTE_FLASH_READ                     = 0x204, //Remote flash read
	REMOTE_FLASH_READ_ANSWER              = 0x804, //Remote flash read answer
	SMARTACK_READ                         = 0x205, //SmartACK read
	SMARTACK_READ_MAILBOX_ANSWER          = 0x805, //SmartACK read mailbox answer
	SMARTACK_READ_LEARNED_SENSOR_ANSWER   = 0x806, //SmartACK read learned sensor answer
	SMARTACK_WRITE                        = 0x206, //SmartACK write
	RC_ACK                                = 0x240, //Remote Commissioning Acknowledge
	RC_GET_METADATA                       = 0x210, //Get Link Table Metadata Query
	RC_GET_METADATA_RESPONSE              = 0x810, //Get Link Table Metadata Response
	RC_GET_TABLE                          = 0x211, //Get Link Table Query
	RC_GET_TABLE_RESPONSE                 = 0x811, //Get Link Table Response
	RC_SET_TABLE                          = 0x212, //Set Link Table Query
	RC_GET_GP_TABLE                       = 0x213, //Get Link Table GP Entry Query
	RC_GET_GP_TABLE_RESPONSE              = 0x813, //Get Link Table GP Entry Response
	RC_SET_GP_TABLE                       = 0x214, //Set Link Table GP Entry Query
	RC_SET_LEARN_MODE                     = 0x220, //Remote Set Learn Mode
	RC_TRIG_OUTBOUND_TEACH_REQ            = 0x221, //Trigger Outbound Remote Teach Request
	RC_GET_DEVICE_CONFIG                  = 0x230, //Get Device Configuration Query
	RC_GET_DEVICE_CONFIG_RESPONSE         = 0x830, //Get Device Configuration Response
	RC_SET_DEVICE_CONFIG                  = 0x231, //Set Device Configuration Query
	RC_GET_LINK_BASED_CONFIG              = 0x232, //Get Link Based Configuration Query
	RC_GET_LINK_BASED_CONFIG_RESPONSE     = 0x832, //Get Link Based Configuration Response
	RC_SET_LINK_BASED_CONFIG              = 0x233, //Set Link Based Configuration Query
	RC_APPLY_CHANGES                      = 0x226, //Apply Changes Command
	RC_RESET_TO_DEFAULTS                  = 0x224, //Reset to Defaults
	RC_RADIO_LINK_TEST_CONTROL            = 0x225, //Radio Link Test Control
	RC_GET_PRODUCT_ID                     = 0x227, //Get Product ID Query
	RC_GET_PRODUCT_RESPONSE               = 0x827, //Get Product ID Response
	RC_GET_REPEATER_FUNCTIONS             = 0x250, //Get Repeater Functions Query
	RC_GET_REPEATER_FUNCTIONS_RESPONSE    = 0x850, //Get Repeater Functions Response
	RC_SET_REPEATER_FUNCTIONS             = 0x251, //Set Repeater Functions Query
	RC_SET_REPEATER_FILTER                = 0x252  //Set Repeater Filter Query
}FCT_CODE;
