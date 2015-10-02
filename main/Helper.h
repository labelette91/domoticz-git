#pragma once

void StringSplit(std::string str, const std::string &delim, std::vector<std::string> &results);
void stdreplace(
	std::string &inoutstring,
	const std::string& replaceWhat, 
	const std::string& replaceWithWhat);
bool file_exist (const char *filename);
std::vector<std::string> GetSerialPorts(bool &bUseDirectPath);
double CalculateAltitudeFromPressure(double pressure);
float pressureSeaLevelFromAltitude(float altitude, float atmospheric, float temp);
float pressureToAltitude(float seaLevel, float atmospheric, float temp);

std::string &stdstring_ltrim(std::string &s);
std::string &stdstring_rtrim(std::string &s);
std::string &stdstring_trim(std::string &s);
double CalculateDewPoint(double temp, int humidity);
uint32_t IPToUInt(const std::string &ip);
bool isInt(const std::string &s);

void sleep_seconds(const long seconds);
void sleep_milliseconds(const long milliseconds);

int mkdir_deep(const char *szDirName, int secattr);

double ConvertToCelsius(const double Fahrenheit);
double ConvertToFahrenheit(const double Celsius);
double ConvertTemperature(const double tValue, const unsigned char tSign);

std::vector<std::string> ExecuteCommandAndReturn(const std::string &szCommand);

std::string To_string (double  val);
std::string To_string (int  val);
std::string To_string (long  val);
std::string To_string (long  long val);
void DateAsciiTotmTime (std::string &sLastUpdate , struct tm &LastUpdateTime  );
void AsciiTime (struct tm &ltime , char * pLastUpdate );
std::string  GetCurrentAsciiTime ();
void AsciiTime ( time_t DateStart, char * DateStr );
time_t DateAsciiToTime_t ( std::string & DateStr );

class CircularBuffer {
public:
  double * Value;
  int Size; //size of buffer'
  int index; //current record index
  double Sum;

  CircularBuffer (int pSize);
  ~CircularBuffer ();
  int GetNext();
  //store value and return last
  double Put(double val);
  //get last value
  double GetLast();      
  void Clear();
  double GetSum();
};

//
class LastValue{
  typedef std::map<int,  double > T_Map_Double_Values;

  T_Map_Double_Values LastValues ;
  double Delta ;
public:

  LastValue(float delta=0);
  double Get(int index);
  void   Put(int index , double value);
  bool AsChanged(int index , double value  );
  bool AsChanged(int index , double value  , double delta );

};
std::string GenerateMD5Hash(const std::string &InputString, const std::string &Salt="");

void hue2rgb(const float hue, int &outR, int &outG, int &outB, const double maxValue = 100.0);
bool is_number(const std::string& s);
void padLeft(std::string &str, const size_t num, const char paddingChar = '0');

bool IsLightOrSwitch(const int devType, const int subType);
