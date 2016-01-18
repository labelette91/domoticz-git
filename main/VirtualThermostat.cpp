#include "stdafx.h"
#include "Helper.h"
#include "Logger.h"
#include "SQLHelper.h"
#include "RFXtrx.h"

#include "VirtualThermostat.h"

#include "mainworker.h"

VirtualThermostat m_VirtualThermostat;

VirtualThermostat::VirtualThermostat()
{
}

VirtualThermostat::~VirtualThermostat()
{
}

  //therlostat mode string
  char * ModeStr[]={
    "Eco",
    "Conf",
    "Frost",
    "Off",
    "Unkn"
  };

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


void VirtualThermostat::ScheduleThermostat(int Minute )
{

    int SwitchValue,nValue,lastSwitchValue;
    std::string RoomTemperatureName ;
    float ThermostatTemperatureSet=0 ;
    const char *ThermostatSwitchName  ;
    struct tm LastUpdateTime ;
    std::string sValue;
    float RoomTemperature=0;
	char * idxThermostat ;
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
try
{	
	//AddjMulti  : value of coef for proportinnal command (PID)
	//AddjValue  ; eco temperature value
	//AddjValue2 ; confor temperature value
  TSqlQueryResult result=m_sql.Query("SELECT Name,nValue,ID,Power,RoomTemp,TempIdx,Type,SubType,sValue,SwitchIdx,AddjMulti,AddjMulti2  FROM DeviceStatus where TempIdx > 0 "  ) ;


	//for all the thermostat switch
	for (unsigned int i=0;i<result.size();i++)
	{
		TSqlRowQuery * row = &result[i] ;
		ThermostatSwitchName		=             (*row)[0].c_str() ;
		lastSwitchValue				=        atoi((*row)[1].c_str() );
		idxThermostat				=     (char *)(*row)[2].c_str() ;
		lastPowerModulation			=        atoi((*row)[3].c_str() );
		lastTemp					= (float)atof((*row)[4].c_str() );
		std::string TemperatureId	=             (*row)[5] ;
		SwitchType					=        atoi((*row)[6].c_str() );
//		SwitchSubType				=        atoi((*row)[7].c_str() );
		SetPoint					=            ((*row)[8].c_str() );
		SwitchIdx					=        atol((*row)[9].c_str() );
		SwitchIdxStr			=        (*row)[9].c_str() ;
		CoefProportional	= (float)atof((*row)[10].c_str() ); //coef for propotianal command PID
		CoefIntegral  		= (float)atof((*row)[11].c_str() ); //coef for propotianal command PID

		ThermostatTemperatureSet= (float)atof(SetPoint);
		ThermostatId = atoi(idxThermostat);	 ;

		if (SwitchIdx<=0){
 				_log.Log(LOG_ERROR,"No Switch device associted to Thermostat  name =%s", ThermostatSwitchName);
		  continue;
		}
		//retrieve corresponding Temperature device name    
		//the temperture corresponding device is stored in LightSubDevice table
		if ( atoi(TemperatureId.c_str()) > 0  )
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
			  TSqlQueryResult resSw = m_sql.Query("SELECT nValue,Type,SubType FROM DeviceStatus WHERE (ID == %s )", SwitchIdxStr.c_str()  ) ;

//				std::string swSubt = m_sql.GetDeviceValue("SubType",SwitchIdxStr.c_str());
//				SwitchSubType = atoi(swSubt.c_str());
				if (resSw.size())
				{
					SwitchSubType    = atoi(resSw[0][2].c_str());
					lastSwitchValue	 = atoi(resSw[0][0].c_str() );
					bool SwitchStateAsChanged = (lastSwitchValue!=SwitchValue) ;


					if ( (Minute % 10 )==0 || (SwitchStateAsChanged))
					{
						if (SwitchValue==1)
							m_mainworker.SwitchLight( SwitchIdx, "On" , 15 , 0 , false, 0, !SwitchStateAsChanged);
	//						m_mainworker.SwitchLight( SwitchIdx, "On" , 15 , 0 , !SwitchStateAsChanged); //ancien prototype sans OOC
						else
	//						m_mainworker.SwitchLight( SwitchIdx, "Off", 0  , 0 , !SwitchStateAsChanged);
							m_mainworker.SwitchLight( SwitchIdx, "Off", 0  , 0 ,  false,0, !SwitchStateAsChanged);
						sleep_milliseconds(1000);
						if (_log.isTraceEnable())	  
							_log.Log(LOG_TRACE,"THER: Mn:%02d  Therm:%-10s(%2s) Room:%4.1f SetPoint:%4.1f Power:%3d LightId(%2ld):%d Kp:%3.f Ki:%3.f Integr:%3.1f",Minute,ThermostatSwitchName, idxThermostat , RoomTemperature,ThermostatTemperatureSet,PowerModulation,SwitchIdx,SwitchValue,CoefProportional,CoefIntegral, DeltaTemps[ThermostatId]->GetSum() /INTEGRAL_DURATION);

					}
					if (( lastPowerModulation != PowerModulation) || (lastTemp != RoomTemperature ) || (SwitchStateAsChanged) )
						m_sql.Query("UPDATE DeviceStatus SET RoomTemp='%4.1f',Power=%d,nValue=%d, LastUpdate='%s' WHERE (ID = %s )", RoomTemperature , PowerModulation ,SwitchValue,  GetCurrentAsciiTime ().c_str(), idxThermostat  	);
													if ((Minute % 10 )==0)
				{
					//compute delta room temperature
					float DeltaTemp = RoomTemperature - Map_LastRoomTemp[ThermostatId] ;
					Map_LastRoomTemp[ThermostatId] = RoomTemperature ;
					if ( DeltaTemp == RoomTemperature ) DeltaTemp=0;  //first call
//						TSqlQueryResult res=m_sql.Query("SELECT ID FROM DeviceStatus where Name=='his_%s'", ThermostatSwitchName ) ;
//						if (res.size() )m_sql.Query( "INSERT INTO Temperature (DeviceRowID, Temperature, Humidity ) VALUES (%s , %4.1f, %d )",res[0][0].c_str(),ThermostatTemperatureSet,PowerModulation	);
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
    TSqlQueryResult result = m_sql.Query("SELECT Time,Temperature FROM SetpointTimers where (DeviceRowID==%s) and ( Time < '%s' ) order by time desc limit 1" ,devID, CurrentTime  ) ;
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
    TSqlQueryResult result = m_sql.Query("SELECT Time,Temperature FROM SetpointTimers where (DeviceRowID==%s) and ( Time > '%s' ) order by time asc limit 1" ,devID, CurrentTime  ) ;
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
  result=m_sql.Query("SELECT MIN(Temperature) FROM SetpointTimers where DeviceRowID==%s" , devID) ;
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
  result=m_sql.Query("SELECT MAX(Temperature) FROM SetpointTimers where DeviceRowID==%s" , devID) ;
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
	if (_log.isTraceEnable()) _log.Log(LOG_TRACE,"THER: Thermostat toggle Idx:%s Temp:%d Duration:%s",devID,targetTemp,Duration  );
}

//return true if in confor mode
std::string VirtualThermostat::GetMode ( float curTemp , float EcoTemp, float ConfTemp )
{
  float  moy=(EcoTemp+ConfTemp)/2;
  if (curTemp<=moy)  return GetModeStr(Eco) ;
  else               return GetModeStr(Confor)  ;
}

/*
*/
bool VirtualThermostat::SetThermostatState(const std::string &idx, const int newState)
{
	return SetMode (idx, (VirtualThermostatMode) newState );
}


char *   VirtualThermostat::GetModeStr( VirtualThermostatMode Mode )
{
  if (Mode>EndMode)
    Mode=EndMode;
    return ModeStr[Mode];
}

VirtualThermostatMode  VirtualThermostat::GetModeInt( const char * mode )
{

  for (int i=0;i<EndMode;i++)
    if (strcmp(mode,ModeStr[i])==0)
      return (VirtualThermostatMode)i;
  return EndMode;

}
bool VirtualThermostat::SetMode ( const std::string &idx,VirtualThermostatMode mode )
{
  if (mode>=EndMode)
    return false;
  if (mode==Confor)
		m_mainworker.SetSetPoint(idx, (float)m_VirtualThermostat.GetConfortTemp(idx.c_str()));
  else   if (mode==Eco)
	  m_mainworker.SetSetPoint(idx, (float)m_VirtualThermostat.GetEcoTemp(idx.c_str()));
  return true;
}
bool VirtualThermostat::SetMode ( const std::string &idx,const char * strMode )
{
  VirtualThermostatMode mode = GetModeInt(strMode);
  return SetMode(idx,mode);
}
std::string VirtualThermostat:: GetAvailableMode() 
{
  std::string AvailableMode;
  for (int i=0;i<EndMode;i++){
    AvailableMode += ModeStr[i] ;
    AvailableMode += ",";
  }
  return AvailableMode ;

}

