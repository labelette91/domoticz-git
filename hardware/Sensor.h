/* ===================================================
 * Sensor.h
 * ====================================================
 * Sensor decoding from 433 Message
 *
 * Created on: 17 sept. 2013
 * Author: disk91 / (c) http://www.disk91.com
 * ===================================================
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#define SENS_CLASS_NONE -1 // Not initialized
#define SENS_CLASS_MTP 0 // MyTeePi virtual or phisical sensor
#define SENS_CLASS_OS 1 // Oregon Scientific

#define SENS_TYP_MTP_CPU 0 // cpu temperature
#define SENS_TYP_MTP_INT 1 // internal temperature

#define SENS_THGR122NX  0x1D20 // THGR122NX
#define SENS_THN132N    0xEC40 // THN132N
#define SENS_THGRN228NX 0x1D30 // THGRN228NX
#define SENS_WGR918     0x3D00 // WGR918       ANEMOMETER
#define SENS_STR928N    0x2D10 // STR928N      RGR918 PLUVIOMETRE
#define SENS_BTHG968    0x5D60 // BTHG968      BTHG968 temperature + hygro + pression atmospherique

#define SENS_POWER      0x3081 // CM180/119
#define SENS_HOMEEASY   0x3B80 // homeeasy



#include <string>
#include "Flag.h"
#include <map>

class Sensor {

 public:
    enum {
    battery		    = 0,
    haveTemperature	= 1,
    haveHumidity	= 2,
    haveBattery		= 3,
    haveChannel		= 4,
    haveDirection	= 5,
    haveSpeed		= 6,
    haveRain		= 7,
    haveTrain		= 8,
    havePressure	= 9,
    haveOnOff       = 10,
    havePower       = 11,
    haveTotal_power = 12,
    isValid		    = 13
  } eInfo;
protected:
  double _temperature;
  double _humidity;
  double _rain;
  double _train;
  double _direction;
  double _speed;
  double _pressure;
  
  int _channel;
  int _irolling ;
  bool _OnOff ;
  int _power ;
  int _total_power ;
  
  bool _valid ; //true if valid
  long _ID;     //iuniq IDentifier 


  int _sensorClass; // marque du sensor cf #define
  int _sensorType; // model of sensor
  std::string _sensorName;

  //store up to 16 bits
  Flags<int> _availableInfos;
  
  // time_t creationTime; // objectCreation time

  virtual bool decode ( char * _str) {return false;} ; // decode the string and set the variable

 protected:
  int getIntFromChar(char c) const ; // transform a Hex value in char into a number
  int getIntFromString(char *) const ; // transform a Hex value in String into a number
  double getDoubleFromString(char *) const ; // transform a BCD string into a double
  

 public:

  Sensor(); // construct and decode value
  Sensor(Sensor &pS ); // construct and decode value

  bool operator!=(const Sensor& ps ) const;

  virtual ~Sensor() { };
  
  bool available(int flag); // return true if valid && parameter flag
  bool availableTemp() const; // return true if valid && have Temp
  bool availableHumidity() const; // return true if valid && have Humidity
  bool isBatteryLow() const; // return true if valid && haveBattery && flag set.
  bool hasChannel() const; // return true if valid && haveChannel
  bool isDecoded() const; // return true if valide
  bool availableSpeed() const; // return true if valid && speed in km/h
  bool availableDirection() const; // return true if valid && wind direction
  bool availableRain() const; // return true if valid && rain in mm/h
  bool availablePressure() const; // return true if valid && pressure in mb

  double getTemperature() const; // return temperature in CÂ°
  double getHumidity() const; // return humidity in % (base 100)
  const std::string& getSensorName() const; // return sensor name
  double getRain() const; // return Rain
  double getTrain() const;
  double getDirection() const; // return wind direction
  double getSpeed() const; // return speed in km/h
  double getPressure() const; // return pressure in mb

  int getChannel() const; // return channel value
  int getSensClass() const; // return sensor class
  int getSensType() const; // return sensor type
  int getSensID() const{return _ID;};   // return sensor ID (rolling code )
  int getOnOff()  { return _OnOff; };   // return switch valur
  bool availableOnOff() const; // return true if valid && have OnOff

  int getPower() { return _power; };   
  int getTotalPower() { return _total_power; };

  //time_t getCreationTime(); // return object creation time

  static Sensor * getRightSensor(char * s); // wrapper for child class

};

class OregonSensorV2 : public Sensor {
 public :
  OregonSensorV2(char * _strval);
  OregonSensorV2();
  static const char _sensorId[];
					  
 private:
  bool decode( char * _str ); // wrapper to right decode method

  bool decode_BTHG968(char *pt); // decode sensor information
  bool decode_RGR918(char *pt); // decode sensor information
  bool decode_THGR122NX(char * pt); // decode sensor informations
  bool decode_THN132N(char * pt); // decode sensor informations
  bool decode_THGRN228NX(char * pt); // decode sensor informations
  bool decode_WGR918(char * pt); // decode sensor informations
  bool validate(char * _str, int _len, int _CRC, int _SUM); // Verify CRC & CKSUM
  bool decode_HOMEEASY(char * pt);
  bool decode_POWER(char * pt);
  
};

class OregonSensorV3 : public Sensor {
 public :
  OregonSensorV3(char * _strval);
  OregonSensorV3();
  static const char _sensorId[];

 private:
  bool decode( char * _str ); // wrapper to right decode method
  bool validate(char * _str, int _len, int _CRC, int _SUM); // Verify CRC & CKSUM

  bool decode_THGR810(char* pt); // decode sensor informations
};



typedef std::map<unsigned long, Sensor*> TSensorMap;

Sensor * FindSensor(unsigned long ID);

extern TSensorMap Sensors;

#endif /* SENSOR_H_ */
