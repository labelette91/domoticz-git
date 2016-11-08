--[[
--]]
 
function setConstants()
	FAN_NAME      = 'VMCCave'    -- exact device name of the switch turning on/off the ventilator
	SENSOR_NAME   = 'Cave'     -- exact device name of the humidity sensor
	EXTERIOR_NAME = 'Exterieur2'     -- exact device name of the extorior humidity sensor
	FAN_TIME_ON   = 'FanTimeOn'
	
end
 
commandArray = {}
 
-- declare some constants

TEST_MODE = false                   -- when true TEST_MODE_HUMVAR is used instead of the real sensor
TEST_MODE_HUMVAR = 'testHumidity'   -- fake humidity value, give it a test value in domoticz/uservars
PRINT_MODE = true					        -- when true wil print output to log and send notifications
SAMPLE_INTERVAL = 10                -- 10 min
TIME_ON = 60                       --temps maximim fan   on 
TIME_OFF = 60                       --temps maximim fan  off

setConstants()

FanTimeOn = tonumber(uservariables[FAN_TIME_ON])

if  (FanTimeOn  == nil) then
    print('Create variables ' .. FAN_TIME_ON .. ' / humCounter / humThresHold integer ' 	)
    commandArray['Variable:'..FAN_TIME_ON] = '0 CREATE STATIC'
    commandArray['Variable:humCounter'] = '0 CREATE STATIC'
    return commandArray
end
 
-- this script runs every minute, humCounter is used to create SAMPLE_INTERVAL periods
humCounter = tonumber(uservariables['humCounter']) 
 
-- increase cycle counter
humCounter = humCounter + 1

if (humCounter < SAMPLE_INTERVAL) then
   commandArray['Variable:humCounter'] = tostring(humCounter)
   return commandArray
end
 
    humCounter = 0 -- reset the cycle counter
		FanTimeOn = FanTimeOn + SAMPLE_INTERVAL
  
-- get the global variables:
humThresHold = tonumber(uservariables['humThresHold'])

-- get the current humidity value
if (TEST_MODE) then
    current = tonumber(uservariables[TEST_MODE_HUMVAR])
else
    current = otherdevices_humidity[SENSOR_NAME]
end
 
-- check if the sensor is on or has some weird reading
if (current == 0 or current == nil) then
    print('current is 0 or nil. Skipping this reading')
    return commandArray
end

extHum = otherdevices_humidity[EXTERIOR_NAME]

lastState = otherdevices[FAN_NAME]
delta = (current - extHum)
 
if (current >= humThresHold) and (delta > 3 ) then
	  if (FanTimeOn >= TIME_OFF ) then
    	newCmd  = 'On'
    else
      newCmd = lastState
    end
else
    newCmd  = 'Off'
end
if ( delta > 10 ) then TIME_ON = TIME_ON * 2 end

if ( lastState == 'On' ) and (FanTimeOn >= TIME_ON ) then
    newCmd  = 'Off'
end

commandArray[FAN_NAME] = newCmd 
if (newCmd ~= lastState) then 	
	FanTimeOn = 0
end

if PRINT_MODE == true then
    print('Current  humidity:' .. current..' Exterior humidity:' .. extHum..  ' humThresHold:' .. humThresHold .. ' FanTimeOn:' .. FanTimeOn .. ' Last Vmc State:' .. lastState ..' New  Vmc State:' .. newCmd )
end
 
-- save the globals
commandArray['Variable:humCounter'] = tostring(humCounter)
commandArray['Variable:FanTimeOn' ] = tostring(FanTimeOn)
 
return commandArray