#pragma once

#include <deque>
#include <list>
#include <string>
#include <fstream>

enum _eLogLevel
{
	LOG_ERROR=0,
	LOG_STATUS=1,
	LOG_NORM=2,
  LOG_TRACE=3
};

enum _eLogFileVerboseLevel
{
	VBL_ERROR=0,
	VBL_STATUS_ERROR=1,
	VBL_ALL=2,
 	VBL_TRACE,

};

class CLogger
{
public:
	struct _tLogLineStruct
	{
		time_t logtime;
		_eLogLevel level;
		std::string logmessage;
		_tLogLineStruct(const _eLogLevel nlevel, const std::string &nlogmessage);
	};

	CLogger(void);
	~CLogger(void);

	void SetOutputFile(const char *OutputFile);
	void SetVerboseLevel(_eLogFileVerboseLevel vLevel);

	void Log(const _eLogLevel level, const char* logline, ...);
	void LogNoLF(const _eLogLevel level, const char* logline, ...);

	void LogSequenceStart();
	void LogSequenceAdd(const char* logline);
	void LogSequenceAddNoLF(const char* logline);
	void LogSequenceEnd(const _eLogLevel level);

	void EnableLogTimestamps(const bool bEnableTimestamps);

	void SetFilterString(std::string  &Filter);
	bool isTraceEnable();
  bool TestFilter(char * cbuffer);
	void setLogVerboseLevel(int LogLevel);
  void SetLogPreference (std::string  LogFilter, std::string  LogFileName , std::string  LogLevel );
  void GetLogPreference ();

	std::list<_tLogLineStruct> GetLog(const _eLogLevel lType);
private:
	boost::mutex m_mutex;
	std::ofstream m_outputfile;
	std::deque<_tLogLineStruct> m_lastlog;
	std::deque<_tLogLineStruct> m_last_status_log;
	std::deque<_tLogLineStruct> m_last_error_log;
	bool m_bInSequenceMode;
	bool m_bEnableLogTimestamps;
	std::stringstream m_sequencestring;
	std::string FilterString;
	std::vector<std::string> FilterStringList;
	std::vector<std::string> KeepStringList;
	_eLogFileVerboseLevel m_verbose_level;
};
extern CLogger _log;
