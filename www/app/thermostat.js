const TypeThermostat = 16 ;

const HTYPE_Dummy = 15 ;

//show thermostat dash board
function ShowTempMobile(item) {
    var status="";

        status += '<label id="bigtext" class="temp" '+ getEcoConforClick(item, 'RefreshFavorites')  +'>' + ShowTargetRoomTemp(item.SetPoint, item.RoomTemp) + '</label>\n' ;
        return status;
}
function ShowTempDownMobile(item) 
{
//  status = '<label id=\"statustext\"> <button class="btn btn-th btn-info" type="button" onmouseup="ThermostatMouseUp()" onmousedown="ThermostatMouseDown(' + item.idx + ',0,\'#dashcontent #light_\');">' + $.t("Dec")  + '</button> </label> ' ;
  status = '<label id=\"statustext\"> <button class="btn btn-th btn-info" type="button" onmouseup="ThermostatMouseUp()" onmousedown="ThermostatMouseDown(' + item.idx + ',0,\'#dashcontent #utility_\');">' + $.t("Dec")  + '</button> </label> ' ;
  return status;
}
function ShowTempUpMobile(item) 
{
//  status =	'<label id=\"img\">        <button class="btn btn-th btn-info" type="button" onmouseup="ThermostatMouseUp()" onmousedown="ThermostatMouseDown(' + item.idx + ',1,\'#dashcontent #light_\');">' + $.t("Inc")  + '</button> </label>';
  status =	'<label id=\"img\">        <button class="btn btn-th btn-info" type="button" onmouseup="ThermostatMouseUp()" onmousedown="ThermostatMouseDown(' + item.idx + ',1,\'#dashcontent #utility_\');">' + $.t("Inc")  + '</button> </label>';
  return status;
}
function ShowThermostatMobile(item) {
    var status="";

    status += ShowTempMobile(item);
//    status += GetThermostatImg(item, "RefreshFavorites",30) ;
    status += GetThermostatMobile(item, "RefreshFavorites",30) ;

//        status +=		'<label id=\"statustext\"> <button class="btn btn-mini btn-info" type="button" onclick="IncrementThermostat(' + item.idx + ',0,\'#dashcontent #light_\');">' + $.t("Dec")  + '</button> </label> ' +
//						'<label id=\"img\">        <button class="btn btn-mini btn-info" type="button" onclick="IncrementThermostat(' + item.idx + ',1,\'#dashcontent #light_\');">' + $.t("Inc")  + '</button> </label>';
    
//        status +=		'<label id=\"statustext\"> <button class="btn btn-th btn-info" type="button" onmouseup="ThermostatMouseUp()" onmousedown="ThermostatMouseDown(' + item.idx + ',0,\'#dashcontent #light_\');">' + $.t("Dec")  + '</button> </label> ' +
//						'<label id=\"img\">        <button class="btn btn-th btn-info" type="button" onmouseup="ThermostatMouseUp()" onmousedown="ThermostatMouseDown(' + item.idx + ',1,\'#dashcontent #light_\');">' + $.t("Inc")  + '</button> </label>';
//
//    status += ShowTempDownMobile(item);
//    status += ShowTempUpMobile(item);


    return status;
}
//thermostat light
function createThermostatSlider(id) {

    //Create Dimmer Sliders
    $(id+' .thslider ').slider({
        //Config
        range: "min",
        min: 1,
        max: 16,
        value: 5,
        step: 1,
        unit: "%",

        //Slider Events
        create: function (event, ui) {
            $(this).slider("option", "max", $(this).data('maxlevel'));
            $(this).slider("option", "min", $(this).data('minlevel'));
            $(this).slider("option", "unit", $(this).data('unit'));
            $(this).slider("value", $(this).data('svalue'));
            $(this).slider("option", "step", $(this).data('step'));
            $(this).slider("option", "tab", $(this).data('tab'));
        },
        slide: function (event, ui) { //When the slider is sliding
            clearInterval($.setDimValue);
            var unit = $(this).slider("option", "unit");
            var maxValue = $(this).slider("option", "max");
            var minValue = $(this).slider("option", "min");
            var tabName  = $(this).slider("option", "tab");
            //    debugger; // This is your breakpoint

            var idx = $(this).data('idx');
            id = "#lightcontent #" + idx;
            id = tabName +  idx; 
            RefreshTargetTemp ( id , ui.value);
            debug("thermostat slider unit " + unit + " tab: " + tabName + " Max:" + maxValue + " Min:" + minValue + "Val:" + ui.value);

            $.setDimValue = setInterval(function () { SetTemperatureValue(idx, ui.value); }, 1000);
            //new refresh
            //clear le timer courant and lauch refresh in 500 ms
//            clearInterval($.myglobals.refreshTimer);
//            $.myglobals.refreshTimer = setInterval(RefreshLights, 1100);
        }
    });

}
function RefreshThSlider(idx, LightId, val) {
    var objSlider = $(LightId + idx + " #ThermostatSlider");
    var oldValue = objSlider.slider("value");
    if (oldValue != val) objSlider.slider("value", val);
    debug("refresh slider idx " + idx + " tab: " + LightId +  "Val:" +val);
}
function ThermostatMouseDown(idx, IncDec, LightId )
{
	$.Inc=0;	
	clearInterval($.IncUpdate);
	IncrementThermostat(idx, IncDec, LightId );
	$.IncUpdate = setInterval(function () { IncrementThermostat(idx, IncDec, LightId ) ; }, 500);
}
function ThermostatMouseUp()
{
	clearInterval($.IncUpdate);
	$.Inc=0;	
}
function IncrementThermostat(idx, IncDec, LightId ) {
//    var objText   = $(LightId + idx + " #bigtext");
    var val  = parseFloat( GetTargetTemp(LightId + idx));
    var val2 = GetRoomTemp(LightId + idx);
    clearInterval($.setDimValue);

//    var val = $(id).slider("option", "value")
//    var maxValue = $(id).slider("option", "max");
//    var minValue = $(id).slider("option", "min");

    //if increment
    if (IncDec == 1) val = val + 0.5; else val = val - 0.5;
    RefreshThSlider(idx, LightId, val);

//    objText.html(BuildStatus(val,val2));
     RefreshTargetTemp ( LightId + idx , val);
    //update server
    $.setDimValue = setInterval(function () { SetTemperatureValue(idx, val); }, 600);
}
function getTextStatus(item) {
    if (item.nValue == 1 )
        return (TranslateStatus("On") + " Power: " + item.Power + " %");
    else
        return (TranslateStatus("Off")+ " Power: " + item.Power + " %");
}
function SetTemperatureValue(idx, value) {
    clearInterval($.setDimValue);

  $.ajax({
		url: "json.htm?type=setused&idx=" + idx + '&setpoint=' + value +  '&used=true' ,
		async: false, 
		dataType: 'json',
		success: function(data) {
		}
	});

}
function mouseDown(idx, value,refreshfunction){
		$.Down=new Date().getTime();
        clearInterval($.Delay);
   		$.Delay = setInterval(function () {showdialogTemperatureHold(idx,value,1,refreshfunction);clearInterval($.Delay); }, 500);

}
function mouseUp(idx, value,refreshfunction){
		var clickTime= new Date().getTime()-$.Down;
        clearInterval($.Delay);
		if (clickTime<1000)
			SetThermostatValue(idx, value, 0,refreshfunction );
		else
			showdialogTemperatureHold(idx,value,1,refreshfunction);
		
}


function getEcoConforClick(item, RefreshLights)
{
var xhtml = 'title="' + $.t("Eco/Confort") + '" onmousedown="mouseDown(' + item.idx + ',' + item.SetPoint + ',' + RefreshLights + ')"  onmouseup="mouseUp(' + item.idx + ',' + item.SetPoint + ',' + RefreshLights + ');" onmouseover="cursorhand()" onmouseout="cursordefault()"'
return xhtml;
}

function GetThermostatMobile(item, RefreshLights, height) {
    var img;
    if 	(item.nValue == 1) {
        img = '<label class="temp" '+ getEcoConforClick(item, RefreshLights) +' style="margin:20 0 20 0px ;" >:ON</label>';
    }
    else {
        img = '<label class="temp" '+ getEcoConforClick(item, RefreshLights) +' style="margin:20 0 20 0px ;" >:OFF</label>';
    }
    return img;
}


//RefreshLights
function GetThermostatImg(item, RefreshLights, height) {
    var img;
    if 	(item.nValue == 1) {
//        img = '<img src="images/heating48_on.png" title="' + $.t("Turn Off") + '" onclick="SwitchLight(' + item.idx + ',\'Off\','+RefreshLights+');" onmouseover="cursorhand()" onmouseout="cursordefault()" height="'+height+'">';
//        img = '<img src="images/heating48_on.png" title="' + $.t("Eco/Confort") + '" onclick="SetThermostatValue(' + item.idx + ','+item.SetPoint+',' + RefreshLights + ');" onmouseover="cursorhand()" onmouseout="cursordefault()" height="' + height + '">';
        img = '<img src="images/smoke48on.png" title="' + $.t("Eco/Confort") + '" onmousedown="mouseDown(' + item.idx + ',' + item.SetPoint + ',' + RefreshLights + ')"  onmouseup="mouseUp(' + item.idx + ',' + item.SetPoint + ',' + RefreshLights + ');" onmouseover="cursorhand()" onmouseout="cursordefault()" height="' + height + '" width="' + height + '" style="vertical-align: middle;">';
    }
    else {
//        img = '<img src="images/heating48_off.png" title="' + $.t("Turn On") + '" onclick="SwitchLight(' + item.idx + ',\'On\',' + RefreshLights + ');" onmouseover="cursorhand()" onmouseout="cursordefault()" height="' + height + '">';
//        img = '<img src="images/heating48_off.png" title="' + $.t("Eco/Confort") + '" onclick="SetThermostatValue(' + item.idx + ','+item.SetPoint+',' + RefreshLights + ');" onmouseover="cursorhand()" onmouseout="cursordefault()" height="' + height + '">';
        img = '<img src="images/smoke48off.png" title="' + $.t("Eco/Confort") + '" onmousedown="mouseDown(' + item.idx + ',' + item.SetPoint + ',' + RefreshLights + ')" onmouseup="mouseUp(' + item.idx + ',' + item.SetPoint + ',' + RefreshLights + ');" onmouseover="cursorhand()" onmouseout="cursordefault()" height="' + height + '" width="' + height + '" style="vertical-align: middle;">';
    }
    return img;
}
function RefreshTargetTemp ( id , val)
{
   		$(id + " #bigtext #targettemp").html(val);
}
function RefreshRoomTemp ( id , val)
{
   		$(id + " #bigtext #roomtemp").html(val);
}
function ShowTargetTemp ( val)
{
var  xhtm= '<label id="targettemp">'+val +'</label>'+ '\u00B0'  ; 
return xhtm;
}
function ShowRoomTemp( val)
{
var  xhtm= '<label id="roomtemp">'+val+'</label>'+ '\u00B0' + $.myglobals.tempsign ; 
return xhtm;
}
function ShowTargetRoomTemp ( Target , Room )
{
var  xhtm= ShowTargetTemp(Target)+'/'+ShowRoomTemp(Room);
return xhtm;
}
function GetTargetTemp(id)
{
    return $(id + " #bigtext #targettemp").html();
}
function GetRoomTemp(id)
{
    return $(id + " #bigtext #roomtemp").html();
}
//display list of temperature device in #comboTemperature from TemperatureList
function RefreshTemperatureComboArray(tabName) {
    //get temperature list
    debug("RefreshTemperatureComboArray");

    $.TemperatureList = [];
    $.ajax({
        url: "json.htm?type=devices&filter=temp&used=true&order=Name",
        async: false,
        dataType: 'json',
        success: function (data) {
            if (typeof data.result != 'undefined') {
                $.each(data.result, function (i, item) {
                    debug("idx:" + item.idx + "name:" + item.Name);
                    $.TemperatureList.push({
                        idx: item.idx,
                        name: item.Name
                    });
                });
            }
        }
    });


    id = tabName + " #comboTemperature";
    var Combo = $(id);
    Combo.find('option').remove().end();

    $.each($.TemperatureList, function (i, item) {
        var option = $('<option />');
        option.attr('value', item.idx).text(item.name);
        Combo.append(option);
    });

    var option = $('<option />');
    option.attr('value', '0').text('');
//    Combo.append(option);

}
//display list of light switch  device in ##combosubdevice from LightsAndSwitches
function RefreshSwitchesComboArray(tabName)
{
//get light switch list
debug("RefreshLightSwitchesComboArray");

	$.LightsAndSwitches = [];
	$.ajax({
		url: "json.htm?type=command&param=getlightswitches", 
		async: false, 
		dataType: 'json',
		success: function(data) {
			if (typeof data.result != 'undefined') {
				$.each(data.result, function(i,item) {
					$.LightsAndSwitches.push({
						idx: item.idx,
						name: item.Name
					});
				});
			}
		}
	});

//add list of switch in the option list
//			var option = $('<option />');
//			option.attr('value', '');
			
    var id = tabName + " #combosubdevice";
    var Combo = $(id);
//	$("#dialog-edittempdevice #combosubdevice")
	$(id)
    .find('option')
    .remove()
    .end()
//	.append(('<option value=""></option>'));
	
	$.each($.LightsAndSwitches, function(i,item){
		var option = $('<option />');
		option.attr('value', item.idx).text(item.name);
		$(id).append(option);
	});

}
function debug(log_txt) {
    if (typeof window.console != 'undefined') {
        console.log("DEBUG: " + log_txt);
    }
}
function SetThermostatValue(idx, value, duration,refreshfunction) {

    clearInterval($.myglobals.refreshTimer);
    ShowNotify($.t('Toggle Eco-Confort') );

    $.ajax({
        url: "json.htm?type=command&param=thermostat&idx=" + idx + "&setTemp=" + value + "&duration=" + value,
        async: false,
        dataType: 'json',
        success: function (data) {
            //wait 1 second
            setTimeout(function () {
                HideNotify();
                refreshfunction();
            }, 1000);
        },
        error: function () {
            HideNotify();
            alert($.t('Problem sending thermostat command'));
        }
    });
}
function setHtmlValue(idx , val ){
  var objText   = $(idx);
  if (objText.html != val) 
   objText.html(val);	
}
function IncTime(sens,idx )
{	
  var objText   = $(idx);
  var val  = parseInt( objText.html());
//  val  = parseInt( idx.innerText);
  //if increment
  if (sens == 1) val = val + 1; else val = val - 1;
  objText.html(val);	
  var slid="#slider"+idx.id;
  $(slid).slider( "value", val );
	$.Inc+=1;	

	if ($.Inc>2)    {
		clearInterval($.IncUpdate);
		$.IncUpdate = setInterval(function () { IncTime(sens,idx) ; }, 250);
		$.Inc=0;	
	}
}
function mDown(sens,idx)
{
	$.Inc=0;	
	clearInterval($.IncUpdate);
	$.IncUpdate = setInterval(function () { IncTime(sens,idx) ; }, 500);
		IncTime(sens,idx);
}
function mUp(obj,idx)
{
	clearInterval($.IncUpdate);
	$.Inc=0;	
}
function SetDateDay()
{
	var timedobj=document.getElementById("Time");
	var obj=document.getElementById("Day");
	if (obj.checked) {
		timedobj.setAttribute('TimeUnit',"day");
	}
	obj=document.getElementById("Hours");
	if (obj.checked) {
		timedobj.setAttribute('TimeUnit',"hr");
	}
}
function SetUnit(unit)
{
	var obj=document.getElementById("Temp");
  obj.setAttribute('TemperatureUnit',unit);
}
function RangeAdjust(obj)
{
		var max = $(obj).slider( "option", "max" );
		var min = $(obj).slider( "option", "min" );
		var value = $(obj).slider( "option", "value" );
		var ivalue=parseInt(value);
		if (ivalue<=min+1) 
			$(obj).slider( "option", "min" , min-5);
		if (ivalue>=max-1) 
			$(obj).slider( "option", "max" , max+5);
}
function createSlider(pTemp,pTime){
				$("#sliderTemp").slider({
				        min: 10,
				        max: 25,
				        value: pTemp,
				        orientation: "vertical",
				    		slide: function( event, ui ) {
				    		    setHtmlValue("#Temp", ui.value);
				    			},
 								change: function( event, ui ) {
 									RangeAdjust(this);
 									}				    			
				    			
				    });
				$("#sliderTime").slider({
				        min: 1,
				        max: 24,
				        value: pTime,
				        orientation: "vertical",
				    		slide: function( event, ui ) {
				    		    setHtmlValue("#Time", ui.value);
				    			},
 								change: function( event, ui ) {
 									RangeAdjust(this);
 									}				    			
				    });
}
function showdialogTemperatureHold(idx,pTemp,pTime,refreshfunction)
{
  $("#dialog-TemperatureHold").dialog({
    autoOpen: false,
    width: "auto",
    height: "auto",
    modal: true,
    resizable: false,
    buttons: {
      "Ok": function () {
          $(this).dialog("close");
          var aTemp = $("#dialog-TemperatureHold  #Temp").html();
          var aTime = $("#dialog-TemperatureHold  #Time").html();
          
          SetThermostatValue(idx, aTemp, aTime,refreshfunction);
          
      },
      Cancel: function () {
        $(this).dialog("close");
      }
    },
    close: function () {
      $(this).dialog("close");
    }
  });

  $("#dialog-TemperatureHold").keydown(function (event) {
    if (event.keyCode == 13) {
      $(this).siblings('.ui-dialog-buttonpane').find('button:eq(0)').trigger("click");
      return false;
    }
  });

  createSlider(pTemp,pTime);
  setHtmlValue("#Temp", pTemp);
  setHtmlValue("#Time", pTime);
  $("#Temp").disableSelection();
  $("#Time").disableSelection();
  $(".btnl").disableSelection();
  $(".btnr").disableSelection();
//	SetUnit('\u00B0'+ $.myglobals.tempsign );
	$("#dialog-TemperatureHold").dialog( "open" );
}
function CancelRefreshTimer ()
{
		if (typeof $scope.mytimer != 'undefined') {
		$interval.cancel($scope.mytimer);
		$scope.mytimer = undefined;
	}
}
function SetLongRefreshTimer ()
{
			$scope.mytimer=$interval(function() {
				RefreshLights();
			}, 10000);
}
function SetShortRefreshTimer ()
{
			$scope.mytimer=$interval(function() {
				RefreshLights();
			}, 100);
}
function getThermostatSlider(idx,TargetTemp , tabName)
{
var xhtm = '<div style="margin-left: 10px;margin-right: 10px;margin-top: 4px;"  class="thslider" id="ThermostatSlider" data-idx="' + idx + '" data-maxlevel="25" data-minlevel="10" data-svalue="' + TargetTemp + '" data-unit="Deg" data-step="0.5"  data-tab="'+ tabName + '"' + '">';
 xhtm += '</div>';
 return xhtm ;
}
function AddLastSeen(item)
{
 item.LastUpdate = $.t('Last Seen') + ':' + item.LastUpdate;
}
function isVirtualThermostat(item)
{
    return ( (item.Type == "Thermostat")&&(item.SubType=="SetPoint") && (item.HwType == HTYPE_Dummy )) ;
}


function ShowTempDown(item,tabName) 
{
 var xhtm='<img   src="images/down.png" style="width:30px;"            height="24px" title="' + $.t('Decrement') + '" onmouseup="ThermostatMouseUp()" onmousedown="ThermostatMouseDown(' + item.idx + ',0,\'' + tabName +'\');" onmouseover="cursorhand()" onmouseout="cursordefault()">';
 return xhtm;
}
function ShowTempUp(item,tabName) 
{
 var xhtm='<img   src="images/up.png"   style="width:30px;"             height="24px" title="' + $.t('Increment') + '" onmouseup="ThermostatMouseUp()" onmousedown="ThermostatMouseDown(' + item.idx + ',1,\'' + tabName +'\');" onmouseover="cursorhand()" onmouseout="cursordefault()">';
 return xhtm;
}


function createUtilityDateSlider()
{
				$("#utilitycontent #sliderDate").slider({
				        min: 0,
				        max: 23*4+3,
				        value: 12*4,
				        orientation: "horizontal",
				    		slide: function( event, ui ) {
				    		    $("#utilitycontent #timerparamstable #combotimehour").val(ui.value>>2);
				    		    $("#utilitycontent #timerparamstable #combotimemin").val(ui.value%4*15);
				    			},
 								change: function( event, ui ) {
 									}				    			
				    			
				    });

				$("#utilitycontent #sliderValue").slider({
				        min: 10,
				        max: 25,
				        value: 20,
				        orientation: "horizontal",
				    		slide: function( event, ui ) {
				    		    $("#utilitycontent #timerparamstable #tvalue").val(ui.value);
				    			},
 								change: function( event, ui ) {
 									}				    			
				    			
				    });

}

function SetTimerHours(hours)
{ 
	$("#utilitycontent #timerparamstable #combotimehour").val(hours);
}
function SetTimerTemp(temp)
{ 
	$("#utilitycontent #timerparamstable #tvalue").val(temp);
}


function RefreshDeviceCombo(ComboName,filter,clear) {
    //get list

    $.List = [];
    $.ajax({
        url: "json.htm?type=devices&filter="+filter+"&used=true&order=Name",
        async: false,
        dataType: 'json',
        success: function (data) {
            if (typeof data.result != 'undefined') {
                $.each(data.result, function (i, item) {
                    debug("idx:" + item.idx + "name:" + item.Name);
                    $.List.push({
                        idx: item.idx,
                        name: item.Name
                    });
                });
            }
        }
    });


    var Combo = $(ComboName);
   
    if (clear) Combo.find('option').remove().end();

    $.each($.List, function (i, item) {
        var option = $('<option />');
        option.attr('value', item.idx).text(item.name);
        Combo.append(option);
    });

    var option = $('<option />');
    option.attr('value', '0').text('');
//    Combo.append(option);

}