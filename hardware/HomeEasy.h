#pragma once

#include "DomoticzHardware.h"
class HomeEasyTransmitter;
class RFM69;

class HomeEasy : public CDomoticzHardwareBase
{
public:
	explicit HomeEasy(const int ID);
	~HomeEasy();
	bool WriteToHardware(const char *pdata, const unsigned char length);
	int TXPIN;
	static int RXPIN;

private:
	bool StartHardware();
	bool StopHardware();
	static void handleInterrupt();

	void Do_Work();
	boost::shared_ptr<boost::thread> m_thread;
	volatile bool m_stoprequested;

	HomeEasyTransmitter *HomeEasyRfTx;

#ifdef WITH_GPIO
	RFM69 * radio;

#endif
};
