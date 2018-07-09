#pragma once

#include "DomoticzHardware.h"

class CDummy : public CDomoticzHardwareBase
{
public:
	explicit CDummy(const int ID);
	~CDummy(void);
	bool WriteToHardware(const char *pdata, const unsigned char length) override;
private:
	void Init();
	bool StartHardware() override;
	bool StopHardware() override;
	void Do_Work();

	std::shared_ptr<std::thread> m_thread;
	volatile bool m_stoprequested;
	int m_ScheduleLastMinute;

};

