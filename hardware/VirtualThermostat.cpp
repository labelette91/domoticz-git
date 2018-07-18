#include "stdafx.h"
#include "../main/Helper.h"
#include "../main/Logger.h"
#include "../main/SQLHelper.h"
#include "../main/RFXtrx.h"

#include "VirtualThermostat.h"

#include "../main/localtime_r.h"
#include "../main/mainworker.h"
#include "../main/ImperiHome.h"

#include <map>

VirtualThermostat * m_VirtualThermostat;

VirtualThermostat::VirtualThermostat(const int ID)
{
	m_HwdID = ID;
	m_VirtualThermostat = this;
	//thermostat mode string
	SetAvailableMode("Eco,Conf,Frost,Off");

}

VirtualThermostat::~VirtualThermostat()
{
	m_bIsStarted = false;
}
bool VirtualThermostat::StartHardware()
{
	m_stoprequested = false;


	//Start worker thread
	m_thread = std::shared_ptr<std::thread>(new std::thread(std::bind(&VirtualThermostat::Do_Work, this)));
	sOnConnected(this);
	m_bIsStarted = true;
	return (m_thread != NULL);
}
bool VirtualThermostat::StopHardware()
{
	m_stoprequested = true;
	if (m_thread != NULL)
		m_thread->join();
	m_bIsStarted = false;
	return true;
}
void VirtualThermostat::Do_Work()
{
	int sec_counter = 0;
	_log.Log(LOG_STATUS, "VirtualThermostat: Worker started...");
	while (!m_stoprequested)
	{
		sleep_milliseconds(1000);
		if (m_stoprequested)
			break;
		sec_counter++;

		if (sec_counter % 12 == 0) {
			m_LastHeartbeat = mytime(NULL);
		}
		time_t atime = mytime(NULL);
		struct tm ltime;
		localtime_r(&atime, &ltime);
		if (ltime.tm_min != m_ScheduleLastMinute)
		{
			ScheduleThermostat(ltime.tm_min);
			m_ScheduleLastMinute = ltime.tm_min;
		}

	}
	_log.Log(LOG_STATUS, "VirtualThermostat: Worker stopped...");
}
bool VirtualThermostat::WriteToHardware(const char *pdata, const unsigned char length)
{
	return true;
}

//time for the power time modulation in minute
#define MODULATION_DURATION 10
#define MODULATION_STEP     10

//guve the output power swicth value in the time from the modulation percent
const short ThermostatOutput[MODULATION_STEP+1][MODULATION_DURATION]={
//           0 1 2 3 4 5 6 7 8 9    
/* 000 % */ { 0,0,0,0,0,0,0,0,0,0 },
/* 010 % */ { 1,0,0,0,0,0,0,0,0,0 },
/* 020 % */ { 1,1,0,0,0,0,0,0,0,0 },
/* 030 % */ { 1,1,1,0,0,0,0,0,0,0 },
/* 040 % */ { 1,1,1,1,0,0,0,0,0,0 },
/* 050 % */ { 1,1,1,1,1,0,0,0,0,0 },
/* 060 % */ { 1,1,1,1,1,1,0,0,0,0 },
/* 070 % */ { 1,1,1,1,1,1,1,0,0,0 },
/* 080 % */ { 1,1,1,1,1,1,1,1,0,0 },
/* 090 % */ { 1,1,1,1,1,1,1,1,1,0 },
/* 100 % */ { 1,1,1,1,1,1,1,1,1,1 },
                    
};                  

//compute the thermostat output switch value 
// the output is modulated in a time period of 10 minute
// each minute , the output is activated dependeing time and percent.
//input : Min : the minute counter 0..59
//input : PowerPercent : the power modulation  0..100%
//output 1 : activated

short VirtualThermostat::ComputeThermostatOutput ( int Min , int PowerPercent )
{
  if (PowerPercent>100) PowerPercent=100;
  //get row value for ThermostatOutput
  PowerPercent = (PowerPercent  ) / MODULATION_STEP ;
  short switchValue = ThermostatOutput[PowerPercent][Min%MODULATION_STEP]; 
  return switchValue;
}                    
  
//return the poqer modulation in function of Room , Exterior and Target Temperature,
int VirtualThermostat::ComputeThermostatPower ( int index , float RoomTemp , float TargetTemp , float ExteriorTemp , float CoefProportional , float CoefIntegral , float CoefDerivated  )
{
  int PowerModulation = 0;
  float DeltaTemp = TargetTemp-RoomTemp;
  CircularBuffer * Delta = DeltaTemps[index];
  if (Delta==0)
  {
    Delta= new CircularBuffer(INTEGRAL_DURATION);
    DeltaTemps[index] = Delta;
  }
  //put last value
  Delta->Put(DeltaTemp);
  //need heating
  if (DeltaTemp>0)
  {
    //PID regulation
    // coef integral is the sum of delta temperature since last 10 minutes.
	  PowerModulation = int(DeltaTemp * CoefProportional + Delta->GetSum() * CoefIntegral /INTEGRAL_DURATION );
  }
	else
		Delta->Clear(); //clear integral part

  if (PowerModulation>100) PowerModulation=100;
  if (PowerModulation<0  ) PowerModulation=0;
  return PowerModulation;
}                    

std::string ToString(float value , char * format)
{
  char buf[1024];
  sprintf(buf,format,value ) ;
  return buf ;
}

void getCommand(std::string &Cmd, std::string &OutCmd, int &level)
{
	OutCmd = Cmd; level = 0;

	if (Cmd == "Off"){
		level = 0;
	}
	else if (Cmd == "On") {
		level = 100;
	}
	else {
		std::vector<std::string> results;
		StringSplit(Cmd, " ", results);
		if (results[0] == "Set") {
			OutCmd = "Set Level"; level = atoi(results[2].c_str()) ;
		}
	}

}
void VirtualThermostat::ScheduleThermostat(int Minute )
{

    int SwitchValue,nValue,lastSwitchValue;
    std::string RoomTemperatureName ;
    float ThermostatTemperatureSet=0 ;
    const char *ThermostatSwitchName  ;
    struct tm LastUpdateTime ;
    std::string sValue,Options;
    float RoomTemperature=0;
		std::string idxThermostat ;
  int ThermostatId ;
	std::string ParentID ;
	int PowerModulation = 0;
	int lastPowerModulation ;
	float lastTemp ;
	int SwitchType;
	int SwitchSubType;
	long  SwitchIdx;
	std::string SwitchIdxStr;
	const char * SetPoint ;
	float CoefProportional , CoefIntegral;
  std::string TemperatureId ;
  std::string OnCmd ;
  std::string OffCmd;
try
{	
//select all the Virtuan device
  TSqlQueryResult result=m_sql.safe_query("SELECT A.Name,A.ID,A.Type,A.SubType,A.nValue,A.sValue, A.Options, B.Name , B.Type     FROM DeviceStatus as A LEFT OUTER JOIN Hardware as B ON (B.ID==A.HardwareID) where (B.type == %d )",HTYPE_VirtualThermostat );

	//for all the thermostat switch
	for (unsigned int i=0;i<result.size();i++)
	{
		TSqlRowQuery * row = &result[i] ;
		ThermostatSwitchName		=             (*row)[0].c_str() ;
		idxThermostat				    =             (*row)[1].c_str() ;
		SwitchType					    =        atoi((*row)[2].c_str() );
		SwitchSubType				    =        atoi((*row)[3].c_str() );
		lastSwitchValue				  =        atoi((*row)[4].c_str() );
		SetPoint					      =            ((*row)[5].c_str() );
		Options					        =            ((*row)[6].c_str() );

    TOptionMap Option   = m_sql.BuildDeviceOptions(Options, false ) ;

    lastPowerModulation =        atoi(Option["Power"     ].c_str());
    lastTemp            = (float)atof(Option["RoomTemp"  ].c_str());
    TemperatureId       =             Option["TempIdx"   ]         ;
    SwitchIdx           =        atol(Option["SwitchIdx" ].c_str());
    SwitchIdxStr        =             Option["SwitchIdx" ]         ;
    CoefProportional    = (float)atof(Option["CoefProp"  ].c_str()); //coef for propotianal command PID
    CoefIntegral        = (float)atof(Option["CoefInteg" ].c_str());        //coef for propotianal command PID
    OnCmd               =             Option["OnCmd"     ]         ;      //switch command for power On the radiator
    OffCmd              =             Option["OffCmd"    ]         ;     //switch command for power Off the radiator

		ThermostatTemperatureSet= (float)atof(SetPoint);
		ThermostatId = atoi(idxThermostat.c_str());	 ;

		if (SwitchIdx<=0){
 				_log.Log(LOG_ERROR,"No Switch device associted to Thermostat  name =%s", ThermostatSwitchName);
		}
		else if (ThermostatTemperatureSet == 0)
		{
			if ((Minute % 10) == 0 )
					_log.Debug(DEBUG_NORM, "VTHER: Mn:%02d  Therm:%-10s(%2s) SetPoint:%4.1f POWER OFF LightId(%2ld):%d ", Minute, ThermostatSwitchName, idxThermostat.c_str(), ThermostatTemperatureSet, SwitchIdx, SwitchValue);

		}
		//retrieve corresponding Temperature device name    
		//the temperture corresponding device is stored in LightSubDevice table
		else if ( atoi(TemperatureId.c_str()) > 0  )
		{
			//get current room temperature  
			if ( m_sql.GetLastValue( TemperatureId.c_str(), nValue, sValue,  LastUpdateTime) )
			{
				RoomTemperature =  m_sql.getTemperatureFromSValue(sValue.c_str());
				PowerModulation = ComputeThermostatPower(ThermostatId,RoomTemperature,ThermostatTemperatureSet,0 ,CoefProportional ,CoefIntegral,0);
				SwitchValue = ComputeThermostatOutput ( Minute,PowerModulation);
				//force to update state in database else only send RF commande with out database update 
//				bool SwitchStateAsChanged = (lastSwitchValue!=SwitchValue) ;
				//if the switch is a HomeEasy protocol with no RF acknoledge , send the RF command each minute with out database DEVICESTATUS table update  
			  TSqlQueryResult resSw = m_sql.safe_query("SELECT nValue,Type,SubType FROM DeviceStatus WHERE (ID == %s )", SwitchIdxStr.c_str()  ) ;

//				std::string swSubt = m_sql.GetDeviceValue("SubType",SwitchIdxStr.c_str());
//				SwitchSubType = atoi(swSubt.c_str());
				if (resSw.size())
				{
					SwitchSubType    = atoi(resSw[0][2].c_str());
					lastSwitchValue	 = atoi(resSw[0][0].c_str() ) != 0 ;

					bool SwitchStateAsChanged = (lastSwitchValue!= SwitchValue) ;


					if ( (Minute % 10 )==0 || (SwitchStateAsChanged))
					{
						std::string  Cmd;	std::string OutCmd; int level;
						if (SwitchValue ==1)
							getCommand(OnCmd , OutCmd, level);
//							m_mainworker.SwitchLight( SwitchIdx, OnCmd , 0 , _tColor(), false, 0 /* , !SwitchStateAsChanged */);
						else
							getCommand(OffCmd, OutCmd, level);
//						m_mainworker.SwitchLight( SwitchIdx, OffCmd, 0  , _tColor(),  false,0 /*, !SwitchStateAsChanged */);
  						m_mainworker.SwitchLight( SwitchIdx, OutCmd, level, _tColor(),  false,0 /*, !SwitchStateAsChanged */);

						sleep_milliseconds(1000);
						_log.Debug(DEBUG_NORM,"VTHER: Mn:%02d  Therm:%-10s(%2s) Room:%4.1f SetPoint:%4.1f Power:%3d LightId(%2ld):%d Kp:%3.f Ki:%3.f Integr:%3.1f Cmd:%s",Minute,ThermostatSwitchName, idxThermostat.c_str() , RoomTemperature,ThermostatTemperatureSet,PowerModulation,SwitchIdx,SwitchValue,CoefProportional,CoefIntegral, DeltaTemps[ThermostatId]->GetSum() /INTEGRAL_DURATION, OutCmd.c_str());

					}
					if (( lastPowerModulation != PowerModulation) || (lastTemp != RoomTemperature ) || (SwitchStateAsChanged) )
          {

            Option["Power"     ] = std::to_string(PowerModulation);
            Option["RoomTemp"  ] = ToString(RoomTemperature,"%4.1f");
						std::string options = m_sql.FormatDeviceOptions(Option, false);
						m_sql.safe_query("UPDATE DeviceStatus SET nValue=%d,Options='%s',LastUpdate='%s' WHERE (ID = %s )", SwitchValue, options.c_str(),TimeToString(NULL, TF_DateTime).c_str(), idxThermostat.c_str());
          }
				if ((Minute % 10 )==0)
				{
					//compute delta room temperature
					float DeltaTemp = RoomTemperature - Map_LastRoomTemp[ThermostatId] ;
					Map_LastRoomTemp[ThermostatId] = RoomTemperature ;
					if ( DeltaTemp == RoomTemperature ) DeltaTemp=0;  //first call
//						TSqlQueryResult res=m_sql.safe_query("SELECT ID FROM DeviceStatus where Name=='his_%s'", ThermostatSwitchName ) ;
//						if (res.size() )m_sql.safe_query( "INSERT INTO Temperature (DeviceRowID, Temperature, Humidity ) VALUES (%s , %4.1f, %d )",res[0][0].c_str(),ThermostatTemperatureSet,PowerModulation	);
				}
				}
				else
					_log.Log(LOG_ERROR,"No switch device associted to Thermostat name =%s", ThermostatSwitchName);

			}
		}
		else
			_log.Log(LOG_ERROR,"No temperature device associted to Thermostat name =%s", ThermostatSwitchName);


	}//nd for
}
catch (...)
{
	_log.Log(LOG_ERROR,"Exception in ScheduleThermostat");
}
}

//return previous  thermostat target temperature before Time
int VirtualThermostat::getPrevThermostatProg ( const char * devID , char * CurrentTime , std::string &Time )
{
  int TargetTemp=0;
    TSqlQueryResult result = m_sql.safe_query("SELECT Time,Temperature FROM SetpointTimers where (DeviceRowID==%s) and ( Time < '%s' ) order by time desc limit 1" ,devID, CurrentTime  ) ;
    if (result.size() ){
      Time       = result[0][0];
      TargetTemp =atoi(result[0][1].c_str());
    }
return TargetTemp;
}

//return next  thermostat target temperature after Time
int VirtualThermostat::getNextThermostatProg ( const char * devID , char * CurrentTime , std::string &Time )
{
  int TargetTemp=0;
    TSqlQueryResult result = m_sql.safe_query("SELECT Time,Temperature FROM SetpointTimers where (DeviceRowID==%s) and ( Time > '%s' ) order by time asc limit 1" ,devID, CurrentTime  ) ;
    if (result.size() ){
      Time       = result[0][0];
      TargetTemp =atoi(result[0][1].c_str());
    }
return TargetTemp;
}

float VirtualThermostat::GetEcoTemp (const char * devID )
{
	std::string temp = m_sql.GetDeviceValue("AddjValue" , devID  );
	return ((float)atof(temp.c_str()));
}

int VirtualThermostat::GetEcoTempFromTimers (const char * devID )
{
  int MinTemp;
  TSqlQueryResult result;
  //get min temperature  from SetpointTimers
  result=m_sql.safe_query("SELECT MIN(Temperature) FROM SetpointTimers where DeviceRowID==%s" , devID) ;
  if (result.size()) MinTemp=atoi(result[0][0].c_str());else MinTemp=16;
  return MinTemp;
}

float VirtualThermostat::GetConfortTemp (const char * devID )
{
	std::string temp = m_sql.GetDeviceValue("AddjValue2" , (devID) );
	return ((float)atof(temp.c_str()));
}

int VirtualThermostat::GetConfortTempFromTimers (const char * devID )
{
  int MaxTemp;
  TSqlQueryResult result;
  //get  max temperature  from SetpointTimers
  result=m_sql.safe_query("SELECT MAX(Temperature) FROM SetpointTimers where DeviceRowID==%s" , devID) ;
  if (result.size()) MaxTemp=atoi(result[0][0].c_str());else MaxTemp=20;
  return MaxTemp;
}

//force : toggle the thermostat temperature state to ECO / CONFORT mode
//algo  if current target temperature = minimum values in Timers table content  then
//      next Target = Maximum values in Timers table content 
//algo  if current target temperature = Maximum values in Timers table content  then
//      next Target = minimum values in Timers table content 

int VirtualThermostat::ThermostatGetEcoConfort (const char * devID , int CurrentTargetTemp , char * Duration)
{
  int NextTargetTemp,MinTemp,MaxTemp;
  TSqlQueryResult result;
  
	MaxTemp = (int)GetConfortTemp(devID);
	MinTemp = (int)GetEcoTemp(devID);

  NextTargetTemp=CurrentTargetTemp;
  int moy=(MaxTemp+MinTemp)/2;
  if (CurrentTargetTemp<=moy)  NextTargetTemp = MaxTemp ;
  else                         NextTargetTemp = MinTemp ;

  return NextTargetTemp;
 
}

void VirtualThermostat::ThermostatToggleEcoConfort (const char * devID , char * setTemp  , char * Duration)
{
	int CurrentTargetTemp =  atoi(setTemp);;
	int targetTemp = ThermostatGetEcoConfort (devID ,CurrentTargetTemp, Duration) ;
	//update the thermostat set point : Cmd Set temp
	 std::string ID  =  devID ;
	m_sql.UpdateDeviceValue("sValue", (int)targetTemp , (ID)) ;
	_log.Debug(DEBUG_NORM,"VTHER: Toggle Eco Confort Idx:%s Temp:%d Duration:%s",devID,targetTemp,Duration  );
}

//return true if in confor mode
std::string VirtualThermostat::GetCurrentMode(TSqlRowQuery * row)
{
 	std::vector<std::string> sValueGlb;
  StringSplit((*row)[sValue], ";", sValueGlb);
  return GetMode ( (float)atof(sValueGlb[0].c_str())  , (float)atof((*row)[AddjValue].c_str()),  (float)atof((*row)[AddjValue2].c_str()) )  ;
};

std::string VirtualThermostat::GetMode ( float curTemp , float EcoTemp, float ConfTemp )
{
  float  moy=(EcoTemp+ConfTemp)/2;
  if (curTemp<=moy)  return "Eco" ;
  else               return "Conf"  ;
}

bool VirtualThermostat::SetThermostatState(const std::string &idx, const int newState)
{
  VirtualThermostatMode mode = (VirtualThermostatMode)newState;
  if (mode>=EndMode)
    return false;
  if (mode==Confor)
		m_mainworker.SetSetPoint(idx, (float)m_VirtualThermostat->GetConfortTemp(idx.c_str()));
  else   if (mode==Eco)
	  m_mainworker.SetSetPoint(idx, (float)m_VirtualThermostat->GetEcoTemp(idx.c_str()));
  else   if (mode == FrostProtection)
	  m_mainworker.SetSetPoint(idx, m_sql.ConvertTemperatureUnit((float)TEMPERATURE_HG));
  else   if (mode == Off)
	  m_mainworker.SetSetPoint(idx, m_sql.ConvertTemperatureUnit((float)TEMPERATURE_OFF));
  return true;
}

//return the thermostat room temperature 
std::string VirtualThermostat::GetRoomTemperature(TSqlRowQuery * row)
{
  return (*row)[RoomTemp] ;
} ;
//return the thermostat setpoint 
std::string VirtualThermostat::GetSetPoint(TSqlRowQuery * row)
{
  return (*row)[sValue] ;
} ;


//convert thermostat option values to map option
TOptionMap VirtualThermostatOptionToOptionMap(
	float Power,
	float RoomTemp,
	float TempIdx,
	float SwitchIdx,
	float EcoTemp,
	float CoefProp,
	float ConforTemp,
	float CoefInteg,
	std::string &OnCmd,
	std::string &OffCmd
	)
{
	TOptionMap Option;

	Option["Power"]      = ToString (Power       , "%4.1f" );
	Option["RoomTemp"]   = ToString (RoomTemp    , "%4.1f" );
	Option["TempIdx"]    = ToString (TempIdx     , "%4.1f" );
	Option["SwitchIdx"]  = ToString (SwitchIdx   , "%4.1f" );
	Option["EcoTemp"]    = ToString (EcoTemp     , "%4.1f" );
	Option["CoefProp"]   = ToString (CoefProp    , "%4.1f" );
	Option["ConforTemp"] = ToString (ConforTemp  , "%4.1f" );
	Option["CoefInteg"]  = ToString (CoefInteg   , "%4.1f" );
	Option["OnCmd"]      = OnCmd;
	Option["OffCmd"]     = OffCmd;

	return Option;
}


//get  thermostat option values from  db
void VirtualThermostatOptionMaptoOptions(TOptionMap &Option,
			float &Power,
			float &RoomTemp,
			float &TempIdx,
			float &SwitchIdx,
			float &EcoTemp,
			float &CoefProp,
			float &ConforTemp,
			float &CoefInteg,
			std::string &OnCmd,
			std::string &OffCmd
			)
	{

		Power = std::stof(Option["Power"]);
		RoomTemp = std::stof(Option["RoomTemp"]);
		TempIdx = std::stof(Option["TempIdx"]);
		SwitchIdx = std::stof(Option["SwitchIdx"]);
		EcoTemp = std::stof(Option["EcoTemp"]);
		CoefProp = std::stof(Option["CoefProp"]);
		ConforTemp = std::stof(Option["ConforTemp"]);
		CoefInteg = std::stof(Option["CoefInteg"]);
		OnCmd = Option["OnCmd"];
		OffCmd = Option["OffCmd"];


	}


void UpdateVirtualThermostatOption(	const uint64_t uidstr,float Power,float RoomTemp,float TempIdx,float SwitchIdx,float EcoTemp,float CoefProp,float ConforTemp,float CoefInteg,std::string &OnCmd,	std::string &OffCmd	)
{


	TOptionMap Option = VirtualThermostatOptionToOptionMap(
		Power,
		RoomTemp,
		TempIdx,
		SwitchIdx,
		EcoTemp,
		CoefProp,
		ConforTemp,
		CoefInteg,
		OnCmd,
		OffCmd
		);

	std::string option = m_sql.FormatDeviceOptions(Option, false);
	m_sql.UpdateDeviceValue("Options", option , std::to_string(uidstr) );
}
std::string VirtualThermostatGetOption (const std::string optionName , const std::string &options )
{
  //if option present
    if (strstr(options.c_str(),optionName.c_str() ) != 0)
    {
      TOptionMap Option   = m_sql.BuildDeviceOptions( options, false ) ;
      return (Option[optionName] );
    }
    else
    {
      return "";
    }
}