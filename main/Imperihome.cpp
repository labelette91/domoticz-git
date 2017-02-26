#include "stdafx.h"
#include <string>
#include "../main/Logger.h"
#include "../main/Helper.h"
#include "../main/SQLHelper.h"

#include "../json/json.h"
#include "../main/RFXtrx.h"
#include "../main/mainworker.h"
#include "../hardware/hardwaretypes.h"
#include "../main/VirtualThermostat.h"
#include "../webserver/Base64.h"

//#define __PI__

#ifndef __PI__
#include "WebServerHelper.h"

extern http::server::CWebServerHelper m_webservers;
#else
#include "WebServer.h"

extern http::server::CWebServer m_webserver;

#endif

enum DeviceTypeEnum
	{
		DevDimmer        =0,
		DevSwitch        ,
		DevTemperature   ,
		DevCamera        ,
		DevCO2           ,
    DevShutter       ,
    DevDoor          ,
    DevFlood         ,
    DevMotion        ,
    DevSmoke         ,
    DevElectricity   ,
    DevGenericSensor ,
    DevHygrometry    ,
    DevLuminosity    ,
    DevLock          ,
    DevMultiSwitch   ,
    DevNoise         ,
    DevPressure      ,
    DevRain          ,
    DevScene         ,
    DevUV            ,
    DevWind          ,
    DevCO2Alert      ,
    DevThermostat    ,
    DevRGBLight			 , // RGB(W) Light (dimmable)
    DevTempHygro		   // Temperature and Hygrometry combined sensor
    
	};

class  ImperiHome {
private:
	Json::Value root;
	Json::Value params;
	std::vector<std::string> sValueGlb;
  std::string nValueGlb ;
  int iroot;


	const char * GetTypeDevice(DeviceTypeEnum dev);
	void ManageAction (std::string &device , std::string &action	 , std::string &actionType	 , std::string actionValue	 );
	DeviceTypeEnum LightType( TSqlRowQuery * row  , Json::Value &params );
	void updateRoot(int ii , TSqlRowQuery * row , DeviceTypeEnum ApType );
	void updateRoot(int ii , TSqlRowQuery * row , DeviceTypeEnum ApType , std::string DevTypeName );
  void updateRoot(int ii , std::string pidx , std::string pname, std::string proom , DeviceTypeEnum ApType , std::string DevTypeName );
	void DeviceContent3(std::string &rep_content);
	void DeviceContent2(std::string &rep_content);
	void DeviceContent1(std::string &rep_content);
  void RoomContent1(std::string &rep_content);
  void SetParams( int KeyNum , const char * KeyName , std::string KeyValue);
  void SetParams( int KeyNum , const char * KeyName , bool KeyValue);
  void SetKey( int KeyNum , const char * KeyName , std::string KeyValue);
  void SetKey( int KeyNum , const char * KeyName , std::string KeyValue , std::string Unit, bool graphable ) ;
	void manageTypeGeneral( TSqlRowQuery * row  , Json::Value &params );
	void Histo(std::string &rep_content, std::string &from );
	void ManageHisto (std::string &device , std::string &value	 , std::string &histo	 , std::string &from	 , std::string &to , std::string &rep_content);
	void getDeviceCamera(int &iroot);
public:
	bool Request( std::string &request_path , std::string &rep_content);
	void getScenes(int &iroot);
  void setGenericSensor(TSqlRowQuery * row) ;
  void setKeyGenericSensor() ;

};

void getGraphic(std::string &idx , std::string TableName , std::string FieldName , std::string KeyName , time_t DateStart , time_t  DateEnd, std::string &rep_content );
	
std::string DeviceTypeString[]={
"DevDimmer",
"DevSwitch",
"DevTemperature",
"DevCamera",
"DevCO2",
"DevShutter",
"DevDoor",
"DevFlood",
"DevMotion",
"DevSmoke",
"DevElectricity",
"DevGenericSensor",
"DevHygrometry",
"DevLuminosity",
"DevLock",
"DevMultiSwitch",
"DevNoise",
"DevPressure",
"DevRain",
"DevScene",
"DevUV",
"DevWind",
"DevCO2Alert",
"DevThermostat",
"DevRGBLight",	
"DevTempHygro"

};	


//graphic table / field 
typedef struct  {
	int			pType;			//type
	std::string Table;			//table name
	std::string Field;			//field name
	std::string KeyNAme;		//key value name
}T_GRAPHIC;

T_GRAPHIC GraphicTable[] = {
	{  },
};

void ImperiHome::setKeyGenericSensor() {
   
  if (nValueGlb!="0")
    SetKey(0,"value" ,nValueGlb    );
  else if (sValueGlb[0].length() !=0)
    SetKey(0,"value" ,sValueGlb[0] );
  else
    SetKey(0,"value" ,nValueGlb    );
}

void ImperiHome::setGenericSensor(TSqlRowQuery * row) {
  setKeyGenericSensor();
  updateRoot( iroot++ , row , DevGenericSensor);
}

const char * ImperiHome::GetTypeDevice(DeviceTypeEnum dev)
{ 
  return DeviceTypeString[dev].c_str();
} 

//index of field in sql query
enum DeviceStatusField
{
ID=0,
Name,
nValue,
Type,
SubType,
sValue,
SwitchType,
LastLevel,
RoomTemp,
SwitchIdx,
AddjValue,
AddjValue2
};


std::string getDeviceId(std::string &device )
{
    //the dev Id is DEVnnn_zzz : nnn is the ID 
 return  device.substr(3,device.find("_")-3);
}

//from to ms since 1/1/1970
void ImperiHome::ManageHisto (std::string &device , std::string &value	 , std::string &histo	 , std::string &from	 , std::string &to , std::string &rep_content )
{
	time_t  DateStartSec ;
	time_t  DateEndSec ;

	_log.Log(LOG_STATUS,"IMPE: Graphic Device:%s Value:%s Histo:%s From:%s To:%s",device.c_str () , value.c_str () , histo.c_str () , from.c_str () , to.c_str ()  );

    //the dev Id is DEVnnn_zzz : nnn is the ID 
    std::string ID = getDeviceId(device);
		//divide by 1000 = sec
		from = from.substr(0,from.length()-3) ;
		to   = to.substr(0,to.length()-3) ;

		DateStartSec= atol(from.c_str());
		DateEndSec	= atol(to.c_str());

//	Histo(rep_content, to )	;
//	getGraphic(ID , "TEMPERATURE" , "Temperature" , "value" , DateStartSec ,  DateEndSec, rep_content );
	//if power graphics 
	if (value=="Watts")
		getGraphic(ID , "Meter"       , "Usage"       , "Watts" , DateStartSec ,  DateEndSec, rep_content );
	else
		getGraphic(ID , "TEMPERATURE", "Temperature"  , "value" , DateStartSec  , DateEndSec, rep_content);

}
void ImperiHome::ManageAction (std::string &device , std::string &action	 , std::string &actionType	 , std::string actionValue	 )
{
      //the dev Id is DEVnnn_zzz : nnn is the ID 
			std::string ID = getDeviceId(device);
			if (_log.isTraceEnable()) _log.Log(LOG_TRACE,"IMPE: Devices:%s Action:%s request:%s Value:%s",device.c_str(), action.c_str(), actionType.c_str() , actionValue.c_str());
			if (actionType=="setStatus")
			{
				if (actionValue=="1" )
					#ifndef __PI__
					m_mainworker.SwitchLight( ID, "On" , "100" , "0", "0" ,0 );
					#else
					m_mainworker.SwitchLight( ID, "On" , "100" , "0" );
					#endif					
				else
					#ifndef __PI__
					m_mainworker.SwitchLight( ID, "Off", "0"   , "0", "0" ,0 );
					#else
					m_mainworker.SwitchLight( ID, "Off", "0" , "0" );
					#endif
			}
			else if (actionType=="setLevel"){
					#ifndef __PI__
					m_mainworker.SwitchLight( ID, "setLevel" , actionValue , "0" , "0",0 );
					#else
					m_mainworker.SwitchLight( ID, "setLevel" , actionValue , "0" );
					#endif
			}
			else if (actionType=="stopShutter"){
			}
			else if (actionType=="pulseShutter"){
			}
			else if (actionType=="launchScene"){
				m_mainworker.SwitchScene(ID, "On" ) ;
			}
			else if (actionType=="setChoice"){
			}
			else if (actionType=="setMode"){
/*
				if (actionValue==ConfMode){
					 m_mainworker.SetSetPoint(ID, (float)m_VirtualThermostat.GetConfortTemp(ID.c_str()));
				}
				else
				if (actionValue==EcoMode){
					 m_mainworker.SetSetPoint(ID, (float)m_VirtualThermostat.GetEcoTemp(ID.c_str()));
				}
				if (actionValue==OffMode){
				}*/
         m_VirtualThermostat.SetMode(ID, actionValue.c_str());


			}
			else if (actionType=="setSetPoint"){
        m_mainworker.SetSetPoint(ID, (float)atof(actionValue.c_str()));
			}

}

void ImperiHome::SetParams( int KeyNum , const char * KeyName , std::string KeyValue)
{
  params[KeyNum][KeyName]   = KeyValue ;
}
void ImperiHome::SetParams( int KeyNum , const char * KeyName , bool KeyValue)
{
  params[KeyNum][KeyName]   = KeyValue ;
}

void ImperiHome::SetKey( int KeyNum , const char * KeyName , std::string KeyValue)
{
  params[KeyNum]["key"]   = KeyName ;
  params[KeyNum]["value"] = KeyValue ;
}

void ImperiHome::SetKey( int KeyNum , const char * KeyName , std::string KeyValue , std::string Unit, bool graphable )
{
  params[KeyNum]["key"]   = KeyName ;
  params[KeyNum]["value"] = KeyValue ;
  if (KeyValue.length())
    SetParams(KeyNum,"unit",Unit);
  if (graphable)
    SetParams(KeyNum,"graphable",graphable);

}

//manage pTypeGeneral
void ImperiHome::manageTypeGeneral( TSqlRowQuery * row  , Json::Value &params )
{

	int dSubType = atoi((*row)[SubType].c_str());
  switch(dSubType)
  {

  case  sTypeVisibility :
    setGenericSensor(row);
    //							float vis = (float)atof(sValue.c_str());
    //							root["result"][ii]["SwitchTypeVal"] = metertype;
    break;
  case  sTypeSolarRadiation :
    setGenericSensor(row);
    //							float radiation = (float)atof(sValue.c_str());
    //							root["result"][ii]["SwitchTypeVal"] = metertype;
    break;
  case  sTypeSoilMoisture :
    setGenericSensor(row);
    //							sprintf(szTmp, "%d cb", nValue);
    //							root["result"][ii]["SwitchTypeVal"] = metertype;
    break;
  case  sTypeLeafWetness :
    setGenericSensor(row);
    //							sprintf(szTmp, "%d", nValue);
    //							root["result"][ii]["SwitchTypeVal"] = metertype;
    break;
  case  sTypeSystemTemp :
    setGenericSensor(row);
    //							double tvalue = ConvertTemperature(atof(sValue.c_str()), tempsign);
    //							root["result"][ii]["Type"] = "temperature";
    break;
  case  sTypePercentage :
    setGenericSensor(row);
    //							sprintf(szData, "%.2f%%", atof(sValue.c_str()));
    //							root["result"][ii]["TypeImg"] = "hardware";
    break;
  case  sTypeFan :
    setGenericSensor(row);
    //							sprintf(szData, "%d RPM", atoi(sValue.c_str()));
    //							root["result"][ii]["Type"] = "Fan";
    break;
  case  sTypeVoltage :
    setGenericSensor(row);
    //							sprintf(szData, "%.3f V", atof(sValue.c_str()));
    //							root["result"][ii]["Voltage"] = atof(sValue.c_str());
    break;
  case  sTypePressure :
    SetKey(0,"value" ,sValueGlb[0] ,"bar" , false);
    updateRoot( iroot++ , row , DevPressure );

    //							sprintf(szData, "%.1f Bar", atof(sValue.c_str()));
    //							root["result"][ii]["Pressure"] = atof(sValue.c_str());
    break;
  case  sTypeSetPoint :
    setGenericSensor(row);
    break;
  case  sTypeTemperature :
    setGenericSensor(row);
    break;
  case  sTypeZWaveClock :
    setGenericSensor(row);
    //								day = atoi(tstrarray[0].c_str());
    //								hour = atoi(tstrarray[1].c_str());
    //								minute = atoi(tstrarray[2].c_str());
    //							root["result"][ii]["DayTime"] = sValue;
    break;
  case  sTypeTextStatus :
    setGenericSensor(row);
    //							root["result"][ii]["Data"] = sValue;//


    break;
  case  sTypeZWaveThermostatMode :
    setGenericSensor(row);
    //							sprintf(szData, "%s", ZWave_Thermostat_Modes[nValue]);
    //							root["result"][ii]["Data"] = szData;
    //							root["result"][ii]["Mode"] = nValue;
    break;
  case  sTypeZWaveThermostatFanMode :
    setGenericSensor(row);

    break;
  case  sTypeAlert :
    setGenericSensor(row);
    //							root["result"][ii]["Data"] = sValue;//

    break;
  case  sTypeCurrent		 :
    setGenericSensor(row);
    break;
  case  sTypeSoundLevel	 :
    setGenericSensor(row);

    break;
  case  sTypeBaro				 :
    SetKey(0,"value" ,sValueGlb[0] ,"bar" , false);
    updateRoot( iroot++ , row , DevPressure );

    break;
  case  sTypeDistance		 :
    setGenericSensor(row);

    break;
  case  sTypeCounterIncremental		 :
    setGenericSensor(row);

    break;
  case  sTypeKwh					 :
    SetKey(0,"Watts"     ,sValueGlb[0] ,"W" ,true );
    SetKey(1,"ConsoTotal",sValueGlb[1] ,"kWh" ,false );
    updateRoot( iroot++ , row , DevElectricity );
    break;
  case  sTypeWaterflow		 :
    setGenericSensor(row);
    break;
  case  sTypeCustom					 :
    setGenericSensor(row);
    break;
  default:
    setGenericSensor(row);
    break;

  }

}

DeviceTypeEnum ImperiHome::LightType( TSqlRowQuery * row  , Json::Value &params )
{
  DeviceTypeEnum ApType = DevSwitch;
	int nVal;
  int dtype = atoi((*row)[SwitchType].c_str());

		switch (dtype)
    {
      case STYPE_OnOff                    :
                                            ApType = DevSwitch;
                                            SetKey(0,"Status",nValueGlb) ;
                                            break;
      case STYPE_Doorbell		              :

      break;
      case STYPE_Contact			            :
                                            ApType = DevGenericSensor;
                                            if (nValueGlb=="1")
																							SetKey(0,"Value" ,"Open");
																						else
																							SetKey(0,"Value" ,"Closed");

      break;
      case STYPE_Blinds			              :
      case STYPE_BlindsInverted           :
                                            ApType = DevShutter;
                                            SetKey(0,"stopable" ,"0");
                                            SetKey(1,"pulseable","0");
                                            SetKey(2,"Level"    , (*row)[LastLevel] );
                                            break;
      case STYPE_X10Siren		              :
      break;
      case STYPE_SMOKEDETECTOR	          :
                                        /*      Armable	Ability to arm the device : 1 = Yes / 0 = No	
				                                        Ackable	Ability to acknowledge alerts : 1 = Yes / 0 = No	
				                                        Armed	Current arming status : 1 = On / 0 = Off
				                                        Tripped	Is the sensor tripped ? (0 = No - 1 = Tripped)			*/
                                            ApType = DevSmoke;
                                            SetKey(0,"Armable" ,"0");
                                            SetKey(1,"Ackable" ,"0");
                                            SetKey(2,"Armed"   ,"1");
                                            SetKey(3,"Tripped" ,nValueGlb);
                                            break;
      case STYPE_Dimmer			              :
                                            ApType = DevDimmer;
																						nVal=atoi(nValueGlb.c_str());
                                            if (nVal==0)
																							SetKey(0,"Status","0");
																						else
																							SetKey(0,"Status","1");

                                            SetKey(1,"level" ,(*row)[LastLevel]);
                                            break;

      case STYPE_Motion			              :
                                            ApType = DevMotion;
                                            SetKey(0,"Armable" ,"0");
                                            SetKey(1,"Ackable" ,"0");
                                            SetKey(2,"Armed"   ,"1");
                                            SetKey(3,"Tripped" ,nValueGlb);
                                            break;
      case STYPE_PushOn			              :
      case STYPE_PushOff			            :
                                            ApType = DevSwitch;
                                            SetKey(0,"Status",nValueGlb) ;
                                            break;
      case STYPE_DoorLock		              :
                                            ApType = DevMotion;
                                            SetKey(0,"Armable" ,"0");
                                            SetKey(1,"Ackable" ,"0");
                                            SetKey(2,"Armed"   ,"1");
                                            SetKey(3,"Tripped" ,nValueGlb);
                                            break;
      case STYPE_Dusk                     :
      break;
      case STYPE_BlindsPercentage         :
      case STYPE_BlindsPercentageInverted :
                                            ApType = DevShutter;
                                            SetKey(0,"stopable" ,"1");
                                            SetKey(1,"pulseable","0");
                                            SetKey(2,"Level"    , (*row)[LastLevel] );
                                            break;
      case STYPE_VenetianBlindsUS	        :
      break;
      case STYPE_VenetianBlindsEU	        :
      break;
			default:
                                            ApType = DevSwitch;
                                            SetKey(0,"Status",nValueGlb) ;
                                            break;

    }
    return ApType;
}

void ImperiHome::updateRoot(int ii , std::string pidx , std::string pname, std::string proom , DeviceTypeEnum ApType , std::string DevTypeName)
{
	  root["devices"][ii]["id"]     = "dev"+pidx+"_" + DevTypeName ;  //devIdx
		root["devices"][ii]["name"]   = pname ;		//Name
		root["devices"][ii]["room"]   = proom ;
		root["devices"][ii]["type"]   = GetTypeDevice(ApType);		//type
		root["devices"][ii]["params"] = params;
}


void ImperiHome::updateRoot(int ii , TSqlRowQuery * row , DeviceTypeEnum ApType ,  std::string DevTypeName  )
{
    updateRoot( ii ,  (*row)[ID] , (*row)[Name], "roomID1" ,  ApType , DevTypeName );
}
void ImperiHome::updateRoot(int ii , TSqlRowQuery * row , DeviceTypeEnum ApType  )
{
    updateRoot( ii ,  (*row)[ID] , (*row)[Name], "roomID1" ,  ApType , "" );

}

void ImperiHome::DeviceContent3(std::string &rep_content)
{
  int dtype,dSubType=0;
  iroot=0;
	root.clear();

  TSqlQueryResult result=m_sql.Query("SELECT ID,Name,nValue,Type,SubType,sValue,SwitchType,LastLevel,RoomTemp,SwitchIdx,AddjValue,AddjValue2  FROM DeviceStatus where (used==1)"  ) ;
	
	for (unsigned int ii=0;ii<result.size();ii++)
	{
		TSqlRowQuery * row = &result[ii] ;
		params.clear();
		dtype    = atoi( (*row)[Type].c_str());
		dSubType = atoi( (*row)[SubType].c_str());
	  
	  StringSplit((*row)[sValue], ";", sValueGlb);
    for (int i=sValueGlb.size();i<5;i++)
      sValueGlb.push_back("");

    nValueGlb = (*row)[nValue] ;


		switch (dtype)
		{
        case pTypeLighting1         :
        case pTypeLighting2         :
        case pTypeLighting3         :
        case pTypeLighting4         :
        case pTypeLighting5         :
        case pTypeLighting6         :
        case pTypeLimitlessLights   :
        case pTypeSecurity1         :
        case pTypeCurtain           :
        case pTypeBlinds            :
        case pTypeRFY               :
        case pTypeChime             :
        case pTypeThermostat2       :
        case pTypeThermostat3       :
        case pTypeRemote            :
        case pTypeRego6XXValue      : 
          updateRoot( iroot++ , row , LightType(  row  , params ) );
          break;
        case pTypeTEMP:	
          SetKey(0,"value" ,sValueGlb[0] , "째C",true );
          updateRoot( iroot++ , row , DevTemperature );
          break;
        case pTypeHUM:	
          SetKey(0,"value" ,nValueGlb ,"%" , false);
          updateRoot( iroot++ , row , DevHygrometry );
          break;
        case pTypeTEMP_HUM :
          //temperature
          //SetKey(0,"value" ,sValueGlb[0] , "째C",true );
          //updateRoot( iroot++ , row , DevTemperature );
          ////humidity
          //SetKey(0,"value" ,sValueGlb[1] ,"%" , false);
          //updateRoot( iroot++ , row , DevHygrometry );
          //DevTempHygro
          SetKey(0,"temp" ,sValueGlb[0] , "째C",true );
          SetKey(1,"hygro",sValueGlb[1] , "%" , false);
          updateRoot( iroot++ , row , DevTempHygro );
          
          break;
        case pTypeTEMP_HUM_BARO :
          //temperature
          SetKey(0,"value" ,sValueGlb[0] , "째C",true );
          updateRoot( iroot++ , row , DevTemperature ,"temp");
          //humidity
          SetKey(0,"value" ,sValueGlb[1] ,"%" , false);
          updateRoot( iroot++ , row , DevHygrometry ,"hum");
          //baro
          SetKey(0,"value" ,sValueGlb[3] ,"mbar" , false);
          updateRoot( iroot++ , row , DevPressure ,"baro");
          break;
        case pTypeTEMP_BARO :
          //temperature
          SetKey(0,"value" ,sValueGlb[0] , "째C",true );
          updateRoot( iroot++ , row , DevTemperature ,"temp");
          //baro
          SetKey(0,"value" ,sValueGlb[1] ,"mbar" , false);
          updateRoot( iroot++ , row , DevPressure ,"baro" );
          break;
        case pTypeUV :
          //UVI
          SetKey(0,"value" ,sValueGlb[0] ,"" , false);
          updateRoot( iroot++ , row , DevUV ,"uv");
          //temperature
          SetKey(0,"value" ,sValueGlb[1] , "째C",false );
          updateRoot( iroot++ , row , DevTemperature ,"temp");
          break;
        case pTypeWIND :
          //wind direction /speed
          SetKey(0,"Direction" ,sValueGlb[0] , "�",false );
          { 
            int intGust = atoi(sValueGlb[3].c_str());
            long double speed = (intGust) *m_sql.m_windscale ;
            SetKey(1,"Speed" , To_string((double)speed) , "km/h",false );
          }
          updateRoot( iroot++ , row , DevWind );
          break;

        case pTypeRAIN :
	          SetKey(0,"Value"     ,sValueGlb[0] ,"mm/h" ,false );
	          SetKey(1,"Accumulation",sValueGlb[1] ,"mm" ,false );
            updateRoot( iroot++ , row , DevRain );

          break;
        case pTypeRFXMeter:
          break;
        case pTypeYouLess :
          break;
        case pTypeP1Power :
          break;
        case pTypeP1Gas :
          break;
        case pTypeCURRENT :
          break;
        case pTypeCURRENTENERGY ://CM180i
        case pTypePOWER :
              //data= I1;I2;I3
	          SetKey(0,"Watts"     ,sValueGlb[0] ,"W" ,false );
            updateRoot( iroot++ , row , DevElectricity ,"i1");
            if (sValueGlb.size()>=2){
	          SetKey(0,"Watts"     ,sValueGlb[1] ,"W" ,false );
            updateRoot( iroot++ , row , DevElectricity ,"i2");
            }
            if (sValueGlb.size()>=3){
	          SetKey(0,"Watts"     ,sValueGlb[2] ,"W" ,false );
            updateRoot( iroot++ , row , DevElectricity ,"i3");
            }
              
          break;
        case pTypeENERGY ://   //CM119/160  //CM180
	          SetKey(0,"Watts"     ,sValueGlb[0] ,"W" ,true );
	          SetKey(1,"ConsoTotal",sValueGlb[1] ,"kWh" ,false );
            updateRoot( iroot++ , row , DevElectricity );
			break;

        case pTypeAirQuality :
	          SetKey(0,"Value"     ,nValueGlb ,"ppm" ,false );
            updateRoot( iroot++ , row , DevCO2 );
        
          break;
        case pTypeThermostat :
          if (dSubType == sTypeThermSetpoint)
          {
//            DevThermostat :
          //temperature
          SetKey(0,"curmode"     ,m_VirtualThermostat.GetMode ( (float)atof(sValueGlb[0].c_str())  , (float)atof((*row)[AddjValue].c_str()),  (float)atof((*row)[AddjValue2].c_str()) )  );
					int idx = atoi ((*row)[SwitchIdx].c_str()) ;//virtual thermostat
					if (idx>0) //virtual thermostat
	          SetKey(1,"curtemp"     ,(*row)[RoomTemp] ,"째C" ,false );
					else
	          SetKey(1,"curtemp"     ,(*row)[sValue] ,"째C" ,false );

          SetKey(2,"cursetpoint" ,(*row)[sValue] );
          SetKey(3,"step"        ,"0.5" );
          SetKey(4,"availablemodes" , m_VirtualThermostat.GetAvailableMode() );
          updateRoot( iroot++ , row , DevThermostat );

          }
          break;
        case pTypeGeneral :
					 manageTypeGeneral(  row  , params ) ;
          break;
        case pTypeGeneralSwitch:
          SetKey(0,"Status",nValueGlb) ;
		  updateRoot(iroot++, row, DevSwitch);
          break;
        case pTypeLux :
          SetKey(0,"value" ,sValueGlb[0] , "lux",false );
          updateRoot( iroot++ , row , DevLuminosity );

          break;
        case pTypeWEIGHT :
          break;
        case pTypeUsage :
          break;
        case pTypeRFXSensor :
          break;


        default:
			setGenericSensor(row);
          break;

    }
	}
  getDeviceCamera(iroot);
	getScenes(iroot);
	rep_content = root.toStyledString();
}

void ImperiHome::getScenes(int &iroot)
{
  Json::Value root;
  #ifndef __PI__
  m_webservers.RType_Scenes(root);
  #else
  m_webserver.RType_Scenes(root);
  #endif	
  if ( root["status"] == "OK" )
  for (unsigned int ii=0;ii<root["result"].size();ii++)
  {
		std::string lastU = root["result"][ii]["LastUpdate"].asString() ;
    params.clear();
		SetKey(0,"LastRun" , lastU);
    updateRoot( iroot++ ,  root["result"][ii]["idx"].asString() , root["result"][ii]["Name"].asString(), "roomID1" , DevScene ,"");
  }
//					root["result"][ii]["idx"] = sd[0];
//					root["result"][ii]["Name"] = sd[1];
//					root["result"][ii]["HardwareID"] = HardwareID;
//					root["result"][ii]["CodeDeviceName"] = CodeDeviceName;
//					root["result"][ii]["Favorite"] = atoi(sd[3].c_str());
//					root["result"][ii]["Protected"] = (iProtected != 0);
//					root["result"][ii]["OnAction"] = onaction;
//					root["result"][ii]["OffAction"] = offaction;
//					root["result"][ii]["Type"] = "Scene";
//					root["result"][ii]["Type"] = "Group";
//					root["result"][ii]["LastUpdate"] = sLastUpdate;
//					root["result"][ii]["Status"] = "Off";
//					root["result"][ii]["Status"] = "On";
//					root["result"][ii]["Status"] = "Mixed";

}



std::string GenerateCamImageURL(std::string address,std::string port,std::string username,std::string password,std::string  imageurl)
{
	std::string feedsrc="http://";
	bool  bHaveUPinURL=(imageurl.find("#USERNAME") != std::string::npos)||(imageurl.find("#PASSWORD") != std::string::npos);

	if (!bHaveUPinURL) {
    if (username!="")
  	{
  		feedsrc+=username+":"+password+"@";
  	}
  }
	feedsrc+=address;
	if (port!="80") {
		feedsrc+=":"+port;
	}
	feedsrc+="/" + imageurl;
	if (bHaveUPinURL) {
		feedsrc=feedsrc.replace(feedsrc.find("#USERNAME"),9,username);
		feedsrc=feedsrc.replace(feedsrc.find("#PASSWORD"),9,password);
	}
	return feedsrc;
}



void ImperiHome::getDeviceCamera(int &iroot)
{

//	var feedsrc=GenerateCamImageURL(csettings.address,csettings.port,csettings.username,csettings.password,csettings.imageurl);


//  		szQuery << "SELECT ID, Name, Enabled, Address, Port, Username, Password, ImageURL FROM Cameras WHERE (Enabled=='1') ORDER BY ID ASC";
//    TSqlQueryResult result=m_sql.Query("SELECT Name,nValue,ID,Type,SubType,sValue,SwitchType,LastLevel,RoomTemp,SwitchIdx,AddjValue,AddjValue2  FROM DeviceStatus where (used==1)"  ) ;
      TSqlQueryResult result=m_sql.Query("SELECT ID, Name, Enabled, Address, Port, Username, Password, ImageURL FROM Cameras WHERE (Enabled=='1') ORDER BY ID ASC"   ) ;
	                                       
	    for (unsigned int ii=0;ii<result.size();ii++)
	    {
		    TSqlRowQuery * sd  = &result[ii] ;
				params.clear();

/*				root["result"][ii]["idx"] = sd[0];
				root["result"][ii]["Name"] = sd[1];
				root["result"][ii]["Enabled"] = (sd[2] == "1") ? "true" : "false";
				root["result"][ii]["Address"] = sd[3];
				root["result"][ii]["Port"] = atoi(sd[4].c_str());
				root["result"][ii]["Username"] = base64_decode(sd[5]);
				root["result"][ii]["Password"] = base64_decode(sd[6]);
				root["result"][ii]["ImageURL"] = sd[7];
*/

        std::string url = GenerateCamImageURL ( (*sd)[3],(*sd)[4],base64_decode((*sd)[5]),base64_decode((*sd)[6]),(*sd)[7]);
//        SetKey(0,"localjpegurl"     , "http://82.188.208.242/axis-cgi/mjpg/video.cgi?camera=&resolution=352x288"  );
//        SetKey(1,"remotemjpegurl"   , "http://82.188.208.242/axis-cgi/mjpg/video.cgi?camera=&resolution=352x288"   );
        SetKey(0,"localjpegurl"     , url  );
        SetKey(1,"remotemjpegurl"   , url   );
				

        updateRoot( iroot++ , sd , DevCamera );
			}

}





void ImperiHome::RoomContent1(std::string &rep_content)
{
		rep_content = "";
		rep_content += "{                                    ";
		rep_content += "  \"rooms\": [                       ";
		rep_content += "    {                                ";
		rep_content += "      \"id\": \"roomID1\",           ";
		rep_content += "      \"name\": \"Living Room\"      ";
		rep_content += "    },                               ";
		rep_content += "    {                                ";
		rep_content += "      \"id\": \"roomID2\",           ";
		rep_content += "      \"name\": \"Kitchen\"          ";
		rep_content += "    },                               ";
		rep_content += "    {                                ";
		rep_content += "      \"id\": \"roomID3\",           ";
		rep_content += "      \"name\": \"room\"      ";
		rep_content += "    }                                ";
		rep_content += "  ]                                  ";
		rep_content += "}                                    ";

}

void ImperiHome::DeviceContent1(std::string &rep_content)
{
			rep_content = "";
			rep_content += "{                                   ";
			rep_content += "  \"devices\": [                    ";
			rep_content += "    {                               ";
			rep_content += "      \"id\": \"dev01\",            ";
			rep_content += "      \"name\": \"My Lamp\",        ";
			rep_content += "      \"type\": \"DevDimmer\",      ";
			rep_content += "      \"room\": \"roomID1\",        ";
			rep_content += "      \"params\": [                 ";
			rep_content += "        {                           ";
			rep_content += "          \"key\": \"Level\",       ";
			rep_content += "          \"value\": \"75\"         ";
			rep_content += "        },                          ";
			rep_content += "        {                           ";
			rep_content += "          \"key\": \"Status\",      ";
			rep_content += "          \"value\": \"1\"          ";
			rep_content += "        }                           ";
			rep_content += "      ]                             ";
			rep_content += "    },                              ";
			rep_content += "    {                               ";
			rep_content += "      \"id\": \"dev02\",            ";
			rep_content += "      \"name\": \"My Lamp2\",        ";
			rep_content += "      \"type\": \"DevDimmer\",      ";
			rep_content += "      \"room\": \"roomID3\",        ";
			rep_content += "      \"params\": [                 ";
			rep_content += "        {                           ";
			rep_content += "          \"key\": \"Level\",       ";
			rep_content += "          \"value\": \"75\"         ";
			rep_content += "        },                          ";
			rep_content += "        {                           ";
			rep_content += "          \"key\": \"Status\",      ";
			rep_content += "          \"value\": \"1\"          ";
			rep_content += "        }                           ";
			rep_content += "      ]                             ";
			rep_content += "    },                              ";
			rep_content += "    {                                                                                         ";
			rep_content += "      \"id\": \"dev03\",                                                                      ";
			rep_content += "      \"name\": \"Temperature Sensor\",                                                       ";
			rep_content += "      \"type\": \"DevTemperature\",                                                           ";
			rep_content += "      \"room\": \"roomID1\",                                                                  ";
			rep_content += "      \"params\": [                                                                           ";
			rep_content += "        {                                                                                     ";
			rep_content += "          \"key\": \"Value\",                                                                 ";
			rep_content += "          \"value\": \"21.5\",                                                                ";
			rep_content += "          \"unit\": \"째C\",                                                                   ";
			rep_content += "          \"graphable\": true                                                                 ";
			rep_content += "        },                                                                                     ";
			rep_content += "      ]                                                                                       ";
			rep_content += "    },                                                                                        ";

			rep_content += "    {                                       ";
			rep_content += "       \"id\": \"dev28\",                   ";
			rep_content += "       \"name\": \"Home Thermostat\",       ";
			rep_content += "       \"type\": \"DevThermostat\",         ";
			rep_content += "       \"room\": \"roomID1\",               ";
			rep_content += "       \"params\": [                        ";
			rep_content += "         {                                  ";
			rep_content += "           \"key\": \"curmode\",            ";
			rep_content += "           \"value\": \"Conf\"           ";
			rep_content += "         },                                 ";
			rep_content += "         {                                  ";
			rep_content += "           \"key\": \"curtemp\",            ";
			rep_content += "           \"value\": \"19.2\",             ";
			rep_content += "           \"unit\": \"째C\"            ";
			rep_content += "         },                                 ";
			rep_content += "         {                                  ";
			rep_content += "           \"key\": \"cursetpoint\",        ";
			rep_content += "           \"value\": \"20\"              ";
			rep_content += "         },                                 ";
			rep_content += "         {                                  ";
			rep_content += "           \"key\": \"step\",               ";
			rep_content += "           \"value\": \"0.5\"               ";
			rep_content += "         },                                 ";
			rep_content += "         {                                  ";
			rep_content += "           \"key\": \"availablemodes\",     ";
			rep_content += "           \"value\": \"Conf, Eco, Off\" ";
			rep_content += "         }                                  ";
			rep_content += "      ]                                     ";
			rep_content += "    },                                       ";  

rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev13\",                                                                      ";
rep_content += "      \"name\": \"Bogus sensor\",                                                             ";
rep_content += "      \"type\": \"DevGenericSensor\",                                                         ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"Blahblah\"                                                             ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Watt\",                                                                 ";
rep_content += "          \"value\": \"1000\"                                                             ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";


			rep_content += "    {                               ";
			rep_content += "      \"id\": \"dev27\",            ";
			rep_content += "      \"name\": \"Light and Conso\",";
			rep_content += "      \"type\": \"DevDimmer\",      ";
			rep_content += "      \"room\": \"roomID2\",        ";
			rep_content += "      \"params\": [                 ";
			rep_content += "        {                           ";
			rep_content += "          \"key\": \"Status\",      ";
			rep_content += "          \"value\": \"1\"          ";
			rep_content += "        },                          ";
			rep_content += "        {                           ";
			rep_content += "          \"key\": \"Level\",       ";
			rep_content += "          \"value\": \"80\"         ";
			rep_content += "        },                          ";
			rep_content += "        {                           ";
			rep_content += "          \"key\": \"Energy\",      ";
			rep_content += "          \"value\": \"349\",       ";
			rep_content += "          \"unit\": \"W\"           ";
			rep_content += "        }                           ";
			rep_content += "      ]                             ";
			rep_content += "    }                               ";
			rep_content += "  ]                                 ";  
			rep_content += "}                                   ";  
}

void ImperiHome::DeviceContent2(std::string &rep_content)
{
rep_content = "";
rep_content += "{                                                                                             ";
rep_content += "  \"devices\": [                                                                            ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev01\",                                                                      ";
rep_content += "      \"name\": \"My Lamp\",                                                                  ";
rep_content += "      \"type\": \"DevDimmer\",                                                                ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Level\",                                                                 ";
rep_content += "          \"value\": \"75\"                                                                   ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Status\",                                                                ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
 rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev02\",                                                                      ";
rep_content += "      \"name\": \"Kitchen Light\",                                                            ";
rep_content += "      \"type\": \"DevSwitch\",                                                                ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Status\",                                                                ";
rep_content += "          \"value\": \"0\"                                                                    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev03\",                                                                      ";
rep_content += "      \"name\": \"Temperature Sensor\",                                                       ";
rep_content += "      \"type\": \"DevTemperature\",                                                           ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"21.5\",                                                                ";
rep_content += "          \"unit\": \"째C\",                                                                   ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev04\",                                                                      ";
rep_content += "      \"name\": \"My Camera\",                                                                ";
rep_content += "      \"type\": \"DevCamera\",                                                                ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"localjpegurl\",                                                          ";
rep_content += "          \"value\": \"http://83.61.22.4:8080/axis-cgi/jpg/image.cgi\"                        ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"remotemjpegurl\",                                                        ";
rep_content += "          \"value\": \"http://83.61.22.4:8080/axis-cgi/mjpg/video.cgi?resolution=320x240\"    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev05\",                                                                      ";
rep_content += "      \"name\": \"Diox sensor\",                                                              ";
rep_content += "      \"type\": \"DevCO2\",                                                                   ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"528\",                                                                 ";
rep_content += "          \"unit\": \"ppm\",                                                                  ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev07\",                                                                      ";
rep_content += "      \"name\": \"Window shutter 1\",                                                         ";
rep_content += "      \"type\": \"DevShutter\",                                                               ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Level\",                                                                 ";
rep_content += "          \"value\": \"75\"                                                                   ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev08\",                                                                      ";
rep_content += "      \"name\": \"Door sensor\",                                                              ";
rep_content += "      \"type\": \"DevDoor\",                                                                  ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"armable\",                                                               ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Armed\",                                                                 ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Tripped\",                                                               ";
rep_content += "          \"value\": \"0\"                                                                    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev09\",                                                                      ";
rep_content += "      \"name\": \"My Flood sensor\",                                                          ";
rep_content += "      \"type\": \"DevFlood\",                                                                 ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"ackable\",                                                               ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Armed\",                                                                 ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Tripped\",                                                               ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev10\",                                                                      ";
rep_content += "      \"name\": \"Motion sensor\",                                                            ";
rep_content += "      \"type\": \"DevMotion\",                                                                ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Armed\",                                                                 ";
rep_content += "          \"value\": \"0\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Tripped\",                                                               ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev11\",                                                                      ";
rep_content += "      \"name\": \"Smoke sensor\",                                                             ";
rep_content += "      \"type\": \"DevSmoke\",                                                                 ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Armed\",                                                                 ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Tripped\",                                                               ";
rep_content += "          \"value\": \"0\"                                                                    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev12\",                                                                      ";
rep_content += "      \"name\": \"Electricity conso\",                                                        ";
rep_content += "      \"type\": \"DevElectricity\",                                                           ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Watts\",                                                                 ";
rep_content += "          \"value\": \"620\",                                                                 ";
rep_content += "          \"unit\": \"W\",                                                                    ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"ConsoTotal\",                                                            ";
rep_content += "          \"value\": \"983\",                                                                 ";
rep_content += "          \"unit\": \"kWh\",                                                                  ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev13\",                                                                      ";
rep_content += "      \"name\": \"Bogus sensor\",                                                             ";
rep_content += "      \"type\": \"DevGenericSensor\",                                                         ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"Blahblah\"                                                             ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev14\",                                                                      ";
rep_content += "      \"name\": \"Hum sensor\",                                                               ";
rep_content += "      \"type\": \"DevHygrometry\",                                                            ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"67\",                                                                  ";
rep_content += "          \"unit\": \"%\",                                                                    ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev15\",                                                                      ";
rep_content += "      \"name\": \"Lum sensor\",                                                               ";
rep_content += "      \"type\": \"DevLuminosity\",                                                            ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"432\",                                                                 ";
rep_content += "          \"unit\": \"lux\",                                                                  ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev16\",                                                                      ";
rep_content += "      \"name\": \"Entrance door Lock\",                                                       ";
rep_content += "      \"type\": \"DevLock\",                                                                  ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Status\",                                                                ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev17\",                                                                      ";
rep_content += "      \"name\": \"Heater\",                                                                   ";
rep_content += "      \"type\": \"DevMultiSwitch\",                                                           ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"Eco\"                                                                  ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Choices\",                                                               ";
rep_content += "          \"value\": \"Eco,Comfort,Freeze,Stop\"                                              ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev18\",                                                                      ";
rep_content += "      \"name\": \"Noise sensor\",                                                             ";
rep_content += "      \"type\": \"DevNoise\",                                                                 ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"33\",                                                                  ";
rep_content += "          \"unit\": \"db\",                                                                   ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev19\",                                                                      ";
rep_content += "      \"name\": \"Outdoor pressure\",                                                         ";
rep_content += "      \"type\": \"DevPressure\",                                                              ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"1432\",                                                                ";
rep_content += "          \"unit\": \"mbar\",                                                                 ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev20\",                                                                      ";
rep_content += "      \"name\": \"Rain sensor\",                                                              ";
rep_content += "      \"type\": \"DevRain\",                                                                  ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"5\",                                                                   ";
rep_content += "          \"unit\": \"mm/h\",                                                                 ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Accumulation\",                                                          ";
rep_content += "          \"value\": \"182\",                                                                 ";
rep_content += "          \"unit\": \"mm\",                                                                   ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev21\",                                                                      ";
rep_content += "      \"name\": \"Night mode\",                                                               ";
rep_content += "      \"type\": \"DevScene\",                                                                 ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"LastRun\",                                                               ";
rep_content += "          \"value\": \"2014-03-12 23:15:65\"                                                  ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev22\",                                                                      ";
rep_content += "      \"name\": \"Away mode\",                                                                ";
rep_content += "      \"type\": \"DevScene\",                                                                 ";
rep_content += "      \"room\": \"roomID1\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"LastRun\",                                                               ";
rep_content += "          \"value\": \"2014-03-16 23:15:65\"                                                  ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev23\",                                                                      ";
rep_content += "      \"name\": \"Sun UV sensor\",                                                            ";
rep_content += "      \"type\": \"DevUV\",                                                                    ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Value\",                                                                 ";
rep_content += "          \"value\": \"5\",                                                                   ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev24\",                                                                      ";
rep_content += "      \"name\": \"Wind sensor\",                                                              ";
rep_content += "      \"type\": \"DevWind\",                                                                  ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Speed\",                                                                 ";
rep_content += "          \"value\": \"12\",                                                                  ";
rep_content += "          \"unit\": \"km/h\",                                                                 ";
rep_content += "          \"graphable\": true                                                                 ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Direction\",                                                             ";
rep_content += "          \"value\": \"182\",                                                                 ";
rep_content += "          \"unit\": \"�\"                                                                     ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev25\",                                                                      ";
rep_content += "      \"name\": \"CO2 Alert sensor\",                                                         ";
rep_content += "      \"type\": \"DevCO2Alert\",                                                              ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"armable\",                                                               ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"ackable\",                                                               ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Armed\",                                                                 ";
rep_content += "          \"value\": \"0\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Tripped\",                                                               ";
rep_content += "          \"value\": \"0\"                                                                    ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev26\",                                                                      ";
rep_content += "      \"name\": \"Outlet and Conso\",                                                         ";
rep_content += "      \"type\": \"DevSwitch\",                                                                ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Status\",                                                                ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Energy\",                                                                ";
rep_content += "          \"value\": \"349\",                                                                 ";
rep_content += "          \"unit\": \"W\"                                                                     ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";
rep_content += "    {                                                                                         ";
rep_content += "      \"id\": \"dev27\",                                                                      ";
rep_content += "      \"name\": \"Light and Conso\",                                                          ";
rep_content += "      \"type\": \"DevDimmer\",                                                                ";
rep_content += "      \"room\": \"roomID2\",                                                                  ";
rep_content += "      \"params\": [                                                                           ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Status\",                                                                ";
rep_content += "          \"value\": \"1\"                                                                    ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Level\",                                                                 ";
rep_content += "          \"value\": \"80\"                                                                   ";
rep_content += "        },                                                                                    ";
rep_content += "        {                                                                                     ";
rep_content += "          \"key\": \"Energy\",                                                                ";
rep_content += "          \"value\": \"349\",                                                                 ";
rep_content += "          \"unit\": \"W\"                                                                     ";
rep_content += "        }                                                                                     ";
rep_content += "      ]                                                                                       ";
rep_content += "    },                                                                                        ";

rep_content += "    {                                       ";
rep_content += "       \"id\": \"dev28\",                   ";
rep_content += "       \"name\": \"Home Thermostat\",       ";
rep_content += "       \"type\": \"DevThermostat\",         ";
rep_content += "       \"room\": \"roomID1\",               ";
rep_content += "       \"params\": [                        ";
rep_content += "         {                                  ";
rep_content += "           \"key\": \"curmode\",            ";
rep_content += "           \"value\": \"Comfort\"           ";
rep_content += "         },                                 ";
rep_content += "         {                                  ";
rep_content += "           \"key\": \"curtemp\",            ";
rep_content += "           \"value\": \"19.2\",             ";
rep_content += "           \"unit\": \"째C\"                 ";
rep_content += "         },                                 ";
rep_content += "         {                                  ";
rep_content += "           \"key\": \"cursetpoint\",        ";
rep_content += "           \"value\": \"20.3\"              ";
rep_content += "         },                                 ";
rep_content += "         {                                  ";
rep_content += "           \"key\": \"step\",               ";
rep_content += "           \"value\": \"0.5\"               ";
rep_content += "         },                                 ";
rep_content += "         {                                  ";
rep_content += "           \"key\": \"availablemodes\",     ";
rep_content += "           \"value\": \"Comfort, Eco, Off\" ";
rep_content += "         }                                  ";
rep_content += "      ]                                     ";
rep_content += "    }                                       ";  

rep_content += "  ]                                 ";  
rep_content += "}                                           ";
}

void ImperiHome::Histo(std::string &rep_content, std::string &from )
{

	int delta=20;
  long fr = atol(from.c_str());
 fr-=10*60*delta; //10 min
rep_content = "";
rep_content += "{                              ";
rep_content += "  \"values\": [                ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + "000,"; fr+=60*delta;
rep_content += "      \"value\" : 18.2         ";
rep_content += "    },                         ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + "000,"; fr+=60*delta;
rep_content += "      \"value\" : 21.3         ";
rep_content += "    },                         ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + "000,"; fr+=60*delta;
rep_content += "      \"value\" : 17.6         ";
rep_content += "    },                         ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + "000,"; fr+=60*delta;
rep_content += "      \"value\" : 23.1         ";
rep_content += "    },                         ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + "000,"; fr+=60*delta;
rep_content += "      \"value\" : 24.9         ";
rep_content += "    },                         ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + "000,"; fr+=60*delta;
rep_content += "      \"value\" : 18.5         ";
rep_content += "    },                         ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + "000,"; fr+=60*delta;
rep_content += "      \"value\" : 20.0         ";
rep_content += "    },                         ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + "000,"; fr+=60*delta;
rep_content += "      \"value\" : 20.1         ";
rep_content += "    },                         ";
rep_content += "    {                          ";
rep_content += "      \"date\" : " + To_string(fr) + ","; fr+=60*delta;
rep_content += "      \"value\" : 19.6         ";
rep_content += "    }                          ";
rep_content += "  ]                            ";   
rep_content += "}																";

}

#include <boost/asio.hpp> 
std::string getIpAdress()
{
	 boost::asio::io_service io_service; 

  boost::asio::ip::tcp::resolver resolver(io_service); 
  boost::asio::ip::tcp::resolver::query query(boost::asio::ip::host_name(), ""); 
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query); 
  boost::asio::ip::tcp::endpoint endpoint = *it; 
	//return endpoint.address().to_string();
	return boost::asio::ip::host_name();
}

//implemtation of ImperiHome Request 
//return false if not a ImperiHome Request
bool  ImperiHome::Request( std::string &request_path , std::string &rep_content)
{



	if (request_path=="/system")
	{
		if (_log.isTraceEnable()) _log.Log(LOG_TRACE,"IMPE: System request"  );
		rep_content = " { \"id\": \"Domoticz"+getIpAdress()+"\",  \"apiversion\": 1 }";
	}
	else  if (request_path.find("/devices")==0)
	{
		std::vector<std::string> results;
		StringSplit(request_path.substr(1),"/",results);
		if (results.size()==1)
		{
			if (_log.isTraceEnable()) _log.Log(LOG_TRACE,"IMPE: Devices request"  );
			DeviceContent3(rep_content);
		}
		else 	if (results.size()==5)
		{
			ManageAction(results[1], results[2], results[3],results[4] );
		}
		else 	if (results.size()==4)
		{
			ManageAction(results[1], results[2], results[3],"" );
		}
		else 	if (results.size()==6)
		{
			ManageHisto(results[1], results[2], results[3],results[4] , results[5] , rep_content);
		}

	}
	else  if (request_path=="/rooms"){
		if (_log.isTraceEnable()) _log.Log(LOG_TRACE,"IMPE: Rooms request"  );
		RoomContent1( rep_content);
	}
	else
		return false;

	return true;
}

bool  ImperiHomeRequest( std::string &request_path , std::string &rep_content)
{
  ImperiHome m_ImperiHome ;
  bool ret = m_ImperiHome.Request( request_path , rep_content);
  if (ret)
	if (_log.isTraceEnable()) 
		if (!_log.TestFilter("IMPA"))
			if (rep_content.length())
			_log.Log(LOG_TRACE, "IMPA: %s", rep_content.c_str());

  return ret;
}

void getGraphic(std::string &idx , std::string TableName , std::string FieldName , std::string KeyName , time_t DateStart , time_t  DateEnd, std::string &rep_content )
{
	TSqlQueryResult result;
	char DateStartStr[40];
	char DateEndStr[40];
	int dType = 0;
	int dSubType = 0;

	
 AsciiTime ( DateStart, DateStartStr );
 AsciiTime ( DateEnd ,  DateEndStr );

rep_content = "";
rep_content += "{";
rep_content += "  \"values\": [";


result = m_sql.Query("SELECT Type,SubType  FROM DeviceStatus where (ID==%s)", idx.c_str());
if (result.size()>0)
{
	TSqlRowQuery * row = &result[0];
	dType = atoi((*row)[0].c_str());
	dSubType = atoi((*row)[1].c_str());
}



result=m_sql.Query ( "SELECT %s , Date FROM %s WHERE (DeviceRowID==%s AND Date>='%s' AND Date<='%s' ) ORDER BY Date ASC" ,FieldName.c_str(), TableName.c_str(),idx.c_str(),DateStartStr,DateEndStr);
for (unsigned int i=0;i<result.size();i++)
{
	TSqlRowQuery * sd = &result[i] ;
	time_t timeSec = DateAsciiToTime_t((*sd)[1]);
	double tvalue = atof((*sd)[0].c_str());

	if ((dType == pTypeGeneral) && (dSubType == sTypeKwh))
		tvalue /= 10.0f;
	else
		tvalue = ConvertTemperature(tvalue, m_sql.m_tempsign[0]);


	char tv[14];
	sprintf (tv ,"%3.1f", tvalue); 
	if (i>0)
			rep_content += ",";
	rep_content += "{";
	rep_content += "\"date\" : " + To_string((int)timeSec) + "000,"; 
	rep_content += "\"value\": " + std::string(tv);
	rep_content += "}";


}
rep_content += "  ]                            ";   
rep_content += "}																";
_log.Log(LOG_STATUS,"IMPE: Graphic Id:%s from:%lu=%s to:%lu=%s Points:%d", idx.c_str(),  (long)DateStart ,DateStartStr, (long)DateEnd,DateEndStr,result.size());

}



  
