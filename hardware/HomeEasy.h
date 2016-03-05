#pragma once

#include "DomoticzHardware.h"
#include "HomeEasyTransmitter.h"
#ifdef __arm__

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

#ifdef __arm__
	SPIClass SPI;
	RFM69 * radio;

#endif
};
