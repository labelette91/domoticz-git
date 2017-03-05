#pragma once

#include "DomoticzHardware.h"
#include <iostream>

class CDummy : public CDomoticzHardwareBase
{
public:
	explicit CDummy(const int ID);
	~CDummy(void);
	bool WriteToHardware(const char *pdata, const unsigned char length);
private:
	void Init();
	bool StartHardware();
	bool StopHardware();
	void Do_Work();

	boost::shared_ptr<boost::thread> m_thread;
	volatile bool m_stoprequested;
	int m_ScheduleLastMinute;

};

