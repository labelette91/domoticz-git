function GetTimersSettings (days , hour,min,val)
{
    //days : bit 0: Monday 1:Tuesday ... 6:  sunday
	var tsettings = {};
    val =  TimerDisplayToDb(val) ;

	tsettings.Active=true;
	tsettings.timertype=2;
	tsettings.date="";
	tsettings.hour=hour;
	tsettings.min=min;
	tsettings.cmd=val;
	tsettings.days=days;
	tsettings.level=0;
	tsettings.hue=0;
    tsettings.Randomness="false"	

	tsettings.mday     ="";
	tsettings.month    ="";
	tsettings.occurence="";

	return tsettings;
}  
//translate the swicth state from database value  to dislay 
//semms to be inverted
function TimerDbToDisplay(dbValue)
{
    if (dbValue==1) return 0 ; else return 1 ;
}
//translate the swicth state from  display to database value
//semms to be inverted
function TimerDisplayToDb(DispValue)
{
    if (DispValue==1) return 0 ; else return 1 ;
}
//read timer set point and store in DayTimer[day][hour]
function getTimers(idx) {
    $.ajax({
        url: "json.htm?type=timers&idx=" + idx,
        async: false,
        dataType: 'json',
        success: function (data) {

            if (typeof data.result != 'undefined') {
                $.each(data.result, function (i, item) {
                    var active = "No";
                    if (item.Active == "true") {

                        var hour = parseInt(item.Time.substring(0, 2));
                        var min = parseInt(item.Time.substring(3, 2));
                        item.Cmd = TimerDbToDisplay(item.Cmd) ;
                        if (item.Type != 5) {
                            var dayflags = parseInt(item.Days);
                            if (dayflags & 0x80) {
                                for (var day = 0; day < 7; day++) setDayTimer(day, hour, item.Cmd);
                            }
                            else if (dayflags & 0x100) {
                                for (var day = 0; day < 5; day++) setDayTimer(day, hour, item.Cmd);
                            }
                            else if (dayflags & 0x200) {
                                for (var day = 5; day < 7; day++) setDayTimer(day, hour, item.Cmd);
                            }
                            else {
                                for (var day = 0; day < 7; day++)
                                    if (dayflags & (1 << day)) setDayTimer(day, hour, item.Cmd);
                            }
                        }
                    }
                });
            }
        }
    });
}
function ClearTimersInt (devIdx)
{
	$.ajax({
		 url: "json.htm?type=command&param=cleartimers&idx=" + devIdx,
		 async: false, 
		 dataType: 'json',
		 success: function(data) {
		 },
		 error: function(){
				ShowNotify($.t('Problem clearing timers!'), 2500, true);
		 }     
	});
}
function ProgAddTimer (devIdx,days,hour,min,val)
{
  if (typeof  val == 'undefined')	return;

  var d="";  for (var i=0;i<7;i++) if (days&(1<<i)) d = d + $.WeekDays[i] +","
  debug ("timer: "+d + " Hour:" + hour + " H Temp:" + val );

	var tsettings=GetTimersSettings(days,hour,min,val);
	$.ajax({
	    
		 url: "json.htm?type=command&param=addtimer&idx=" + $.devIdx + 
					"&active=" + tsettings.Active + 
					"&timertype=" + tsettings.timertype +
					"&date=" + tsettings.date +
					"&hour=" + tsettings.hour +
					"&min=" + tsettings.min +
					"&randomness=" + tsettings.Randomness +
					"&command=" + tsettings.cmd +
					"&level=" + tsettings.level +
					"&hue=" + tsettings.hue +
					"&days=" + tsettings.days +
					"&mday=" + tsettings.mday +
					"&month=" + tsettings.month +
					"&occurence=" + tsettings.occurence,
	    
		 async: false, 
		 dataType: 'json',
		 success: function(data) {
		 },
		 error: function(){
				ShowNotify($.t('Problem addsetpointtimer timers!'), 2500, true);
		 }     
	});
}
function ProgAddTimersBtn(devIdx) {
	ClearTimersInt(devIdx);
	var lastValue = [];
	for (var hour = 0; hour < 24; hour++) {
		var tdays = 0;
		var lastHourValue = DayTimer[0][hour];
		var DayHourValue = 0;
		for (day = 0; day < 7; day++) {
			var Value = DayTimer[day][hour];
			if (Value != lastValue[day]) {
				if (lastHourValue != Value) {
					ProgAddTimer(devIdx, tdays, hour, 0, lastHourValue);
					tdays = 0;
					lastHourValue = Value;
				}
				tdays |= (1 << day);
				lastValue[day] = Value;
				DayHourValue = Value;
			}
		}
		if (tdays != 0)
			ProgAddTimer(devIdx, tdays, hour, 0, DayHourValue);
	}
	//            $.each($("button.btn-timer."+entry), function(i,item) {CreateTimer(devIdx,day,i,item);	});        
	ShowNotify($.t('Sensor Timer added!'), 2500, true);
}
//update the DAY/hours timer data array value
function setDayTimer(day,hour,value)
{
var undef;
	if (typeof value == "string"){
		if (value!="")
			DayTimer[day][hour]=parseInt(value);
		else
			DayTimer[day][hour]=undef;
	}
	else
		DayTimer[day][hour]=value;
    
}
//get the DAY/hours timer data array value
function getDayTimer(day,hour)
{
return DayTimer[day][hour] ;
    
}
function SetOnBkgd(obj){
    obj.removeClass("btn-info");
    obj.addClass("btn-confor");
}
function SetOffBkgd(obj){
    obj.removeClass("btn-confor");
    obj.addClass("btn-info");

}
function SetNoneBkgd(obj){
	obj.removeClass("btn-info");
	obj.removeClass("btn-confor");
}
function SetBkgd(obj,value){

  if (value==getOnVal() ){
    SetOnBkgd(obj);
  }
  else if (value==getOffVal() ){
    SetOffBkgd(obj)
  }
  else {
      SetNoneBkgd(obj);
  }

}
function UpdateButtonHmi( day,hour,value)
{
    var obj = getItem(day,hour);
    setDisplayValue(obj, value);
    SetBkgd(obj,value);
}
//update the display hmi day/hour button and data array value
function SetDayHourValue( day,hour,value)
{
	var obj = getItem(day,hour);
    //toggle value
	if ( ( value!=1 )&& ( value!=0 ) ){
	    value = getDayTimer(day,hour);
	    if (typeof value == 'undefined') value = 0;
	    if (value == 0) value = 1; else value = 0;
    }
    UpdateButtonHmi( day,hour,value);	
	setDayTimer(day,hour,value)
}
//update the display hmi of hour button for alol selected day
function SetHoursValue( dayN,hour,value)
{ 
	if (istSelectedDay()){
         SetDayHourValue( dayN,hour,value);
	}
	else{
	//selection using row and day	checkbox
     for (var day=0;day<7;day++) {
        var entry = $.WeekDays[day];
        if ( getDayCheckBox(entry).is(":checked")) 
        {
            SetDayHourValue( day,hour,value);
		}
     };
	}
	DisplayTimerValues();
}
function SetDays (TypeStr, bDisabled)
{
		getDayCheckBox("Mon").prop('checked', ((TypeStr.indexOf("Mon") >= 0)||(TypeStr=="Everyday")||(TypeStr=="Weekdays")) ? true : false);
		getDayCheckBox("Tue").prop('checked', ((TypeStr.indexOf("Tue") >= 0)||(TypeStr=="Everyday")||(TypeStr=="Weekdays")) ? true : false);
		getDayCheckBox("Wed").prop('checked', ((TypeStr.indexOf("Wed") >= 0)||(TypeStr=="Everyday")||(TypeStr=="Weekdays")) ? true : false);
		getDayCheckBox("Thu").prop('checked', ((TypeStr.indexOf("Thu") >= 0)||(TypeStr=="Everyday")||(TypeStr=="Weekdays")) ? true : false);
		getDayCheckBox("Fri").prop('checked', ((TypeStr.indexOf("Fri") >= 0)||(TypeStr=="Everyday")||(TypeStr=="Weekdays")) ? true : false);
		getDayCheckBox("Sat").prop('checked', ((TypeStr.indexOf("Sat") >= 0)||(TypeStr=="Everyday")||(TypeStr=="Weekends")) ? true : false);
		getDayCheckBox("Sun").prop('checked', ((TypeStr.indexOf("Sun") >= 0)||(TypeStr=="Everyday")||(TypeStr=="Weekends")) ? true : false);
		getDayCheckBox("Mon").attr('disabled', bDisabled);
		getDayCheckBox("Tue").attr('disabled', bDisabled);
		getDayCheckBox("Wed").attr('disabled', bDisabled);
		getDayCheckBox("Thu").attr('disabled', bDisabled);
		getDayCheckBox("Fri").attr('disabled', bDisabled);
		getDayCheckBox("Sat").attr('disabled', bDisabled);
		getDayCheckBox("Sun").attr('disabled', bDisabled);
}
function SetVal(id,temp)
{ 
	$(id).val(temp);
}
function debug(log_txt) {
    if (typeof window.console != 'undefined') {
        console.log("DEBUG: " + log_txt);
}
}
function DisplayTimerValues()
{
     for (day=0;day<7;day++) {
		var temp=0;
        for ( var hour = 0; hour < 24; hour++ ) {
          var obj = getItem(day,hour);
          value=getDayTimer(day,hour);
		  if (typeof  value == 'undefined'){
			value = temp;
			SetBkgd(obj,value);
        }
		  else{
			temp=value;
			setDisplayValue(obj,value);
			SetBkgd(obj,value);
        }
		}
     };
}
//entry = Mon Tue... : read checkbox On
function getDayCheckBox(entry)
{
  return $("#lightcontent #"+entry+" #Chk");
}
//day: 0..6
function getItem(day,hour)
{
  var entry = $.WeekDays[day];
  var obj = $("#lightcontent #"+entry+" #"+hour);
  return obj;
}
function getOnVal()
{
  return 1  ;
}
function getOffVal()
{
  return 0 ;
}
function setDisplayValue(obj,value)
{
    if (value==1) $(obj).html("On") ; 
    else if (value == 0) $(obj).html("Off");
    else $(obj).html("");
}
function GetCmdValue()
{
return $.GlobalValue;
}
function SetCmdValue(value) {
     $.GlobalValue=value;
}
function clearDayTimer()
{
	 for (var i=0;i<7;i++) DayTimer[i]=[];
}
function istSelectedDay()
{
  return $("#lightcontent #when4").prop('checked');
}
function clearDisplay()
{
     for (day=0;day<7;day++) {
        for ( var hour = 0; hour < 24; hour++ ) {
          var obj = getItem(day,hour);
          $(obj).html('') ; 
          SetNoneBkgd(obj);
		};
     };
}
function SetBtn(obj)
{
    SetNoneBkgd($('#lightcontent #BtnOn'))
    SetNoneBkgd($('#lightcontent #BtnOff'))
    SetNoneBkgd($('#lightcontent #BtnToggle'))
//    SetOnBkgd(obj)
    $(obj).addClass("btn-confor");

    value = -1;
    if ($(obj).html() == "On") value = 1;
    if ($(obj).html() == "Off") value = 0;
//    $("#lightcontent #combocommand").val(value);
    SetCmdValue(value);
//    var cb = $("#lightcontent #combocommand");
//    cb.options[cb.selectedIndex].text;    //text de l'option = affiche
//    cb.options[cb.selectedIndex].value;   //valeur de l'option = pas affichée affiche
//    cb.text() //the text content of the selected option
//    cb.val() //the value of the selected option
    
}
function ShowIhmTimersInt(devIdx, name, isdimmer, stype, devsubtype)
{
  $.MouseDown = false ;
  $.DayDeb = 0 ;
  $.DayEnd = 0 ;
  $.HourDeb = 0 ;
  $.HourEnd = 0 ;
  $.GlobalValue = 1 ;
  //init button On	
  SetBtn($('#lightcontent #BtnOn'));

  $.WeekDays = ["Mon","Tue","Wed","Thu","Fri","Sat","Sun"]; 
  DayTimer  = new Array(7); 
  clearDayTimer() ;
  getTimers(devIdx);
  DisplayTimerValues();
  
  $("button.btn-timer").mouseover(function () {
     if ($.MouseDown){
//        SetHoursValue( $(this).prop("name"),$(this).prop("id"), GetCmdValue() );  
         $.DayCur = $(this).prop("name");
         $.HourCur = $(this).prop("id");
         if (istSelectedDay()) {
             for (var day = $.DayDeb; day <= $.DayCur; day++)
                 for (var hour = $.HourDeb; hour <= $.HourCur; hour++)
                     if ((day != $.DayDeb) || (hour != $.HourDeb))
                         SetOnBkgd(getItem(day, hour));
         }
     }
  });
  
  $("button.btn-timer").mousedown(function () {
    $.MouseDown = true ;
	$.DayDeb = $(this).prop("name") ;
	$.HourDeb = $(this).prop("id") ;
    SetHoursValue($.DayDeb,$.HourDeb , GetCmdValue() );  
  });
  
  $("button.btn-timer").mouseup(function () {
    $.MouseDown = false ;
	$.DayEnd = $(this).prop("name") ;
	$.HourEnd = $(this).prop("id") ;
	if (istSelectedDay()){
		for (var day=$.DayDeb;day<=$.DayEnd;day++)
			for (var hour=$.HourDeb;hour<=$.HourEnd;hour++)
			    if ( (day != $.DayDeb)||(hour!=$.HourDeb))
			        SetDayHourValue(day, hour, GetCmdValue());
	}
  });
  
  $(window).mouseup(function(){
    $.MouseDown = false ;
  });
  $("#lightcontent #when1").click(function() {
  	SetDays("Everyday",false);                    
  });                                                              
  $("#lightcontent #when2").click(function() {
  	SetDays("Weekdays",false);                    
  });                                                              
  $("#lightcontent #when3").click(function() {
  	SetDays("Weekends",false);                    
  });                                                              
  $("#lightcontent #when4").click(function() {
  	SetDays("",false);                           
  });         

   $("#lightcontent #tConf").click(function() {
    
  });                                                              
   $("#lightcontent #tEco").click(function() {
   
  });                                                              

/*
    $("#lightcontent #combocommand").change(function() {
    	var cval=$("#lightcontent #combocommand").val()
    	SetCmdValue( cval);
    });*/

}
function createDayHourTable()
{
    var Days = ["Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"]; 
    var WeekDays = ["Mon","Tue","Wed","Thu","Fri","Sat","Sun"]; 

    var html=
    '<table BORDER="0">\n'+
    '<tr id="time">    \n'+                                       
    '<td></td>         \n';
    for (var i=0;i<24;i++)
      html+= '<td><button id="'+i+'"  class="btn  btn-timer" type="button"  >'+i+'</button> </td>\n';                                  
    html+="</tr>\n";
    
    for (var day= 0;day<7;day++) 
    {
        html+='<tr id="'+WeekDays[day]+'">\n';
        html+='<td><input type="checkbox" id="Chk" checked/>&nbsp;<span data-i18n="'+Days[day]+'">'+Days[day]+'</span>&nbsp;</td>\n';

        for (var hour=0;hour<24;hour++)
        {
            html+='<td><button id="'+hour+'" name="'+day+'" class="btn  btn-timer " type="button"  ></button></td>\n';
        }
        html+='</tr>\n';
    }
    html+='</table>\n';
    return html;
}
function ProgCopyTimersBtn(idx)
{
			$.devIdx=idx;
			$( "#dialog-copy" ).dialog({
				  autoOpen: false,
				  width: 400,
				  height: 160,
				  modal: true,
				  resizable: false,
				  buttons: {
					  "OK": function() {
						  var bValid = true;
						  
							var SensorIdx =$("#dialog-copy #sensor option:selected").val();
							var SensorName=$("#dialog-copy #sensor option:selected").text();
							$(this).dialog("close");
							if (typeof SensorName == 'undefined') {
								bootbox.alert($.t('No Sensor Type Selected!'));
								return ;
							}
							clearDayTimer() ;
							clearDisplay();
							getTimers(SensorIdx);
							DisplayTimerValues();

							bootbox.alert($.t('Sensor Timer '+ SensorName +' copied!'));

					  },
					  Cancel: function() {
						  $( this ).dialog( "close" );
					  }
				  },
				  close: function() {
					$( this ).dialog( "close" );
				  }
			});
			RefreshDeviceCombo("#dialog-copy #sensor", "light", true);
			$( "#dialog-copy" ).i18n();
			$( "#dialog-copy" ).dialog( "open" );
}


