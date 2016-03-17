#pragma once

#include "DomoticzHardware.h"
#include "HomeEasyTransmitter.h"
#ifdef __arm__

#define SS 0
#define RF69_IRQ_PIN 0 
#define RF69_IRQ_NUM 0 
#include "RFM69.h"
#include "RFM69registers.h"

#endif
class HomeEasy : public CDomoticzHardwareBase
{
public:
	explicit HomeEasy(const int ID);
	~HomeEasy();
	bool WriteToHardware(const char *pdata, const unsigned char length);
private:
	bool StartHardware();
	bool StopHardware();

	void Do_Work();
	boost::shared_ptr<boost::thread> m_thread;
	volatile bool m_stoprequested;

	HomeEasyTransmitter *HomeEasyRfTx;
	int TXPIN ;
	int RXPIN ;

#ifdef __arm__
	RFM69 * radio;

#endif
};
