function GetSetpointSettings (days , hour,min,val)
{
    //days : bit 0: Monday 1:Tuesday ... 6:  sunday
	var tsettings = {};
	tsettings.Active=true;
	tsettings.timertype=2;
	tsettings.date="";
	tsettings.hour=hour;
	tsettings.min=min;
	tsettings.tvalue=val;
	tsettings.days=days;
	return tsettings;
}  

//read timer set point and store in DayTimer[day][hour]
function getTimerSetPoints(idx)
{
   $.ajax({
			 url: "json.htm?type=setpointtimers&idx=" + idx, 
			 async: false, 
			 dataType: 'json',
			 success: function(data) {
				
			  if (typeof data.result != 'undefined') {
				$.each(data.result, function(i,item){
					var active="No";
					if (item.Active == "true") {
				
            var hour = parseInt(item.Time.substring(0,2));
            var min  = parseInt(item.Time.substring(3,2));
            
  					if (item.Type!=5) 
  					{
  						var dayflags = parseInt(item.Days);
  						if (dayflags & 0x80){
                for (var day=0;day<7;day++)  setDayTimer(day,hour,item.Temperature);
  						}
  						else if (dayflags & 0x100){                
                for (var day=0;day<5;day++)  setDayTimer(day,hour,item.Temperature);
              }
  						else if (dayflags & 0x200){            
                for (var day=5;day<7;day++)  setDayTimer(day,hour,item.Temperature);
              }
  						else {
  						  for (var day=0;day<7;day++)                                   
  							  if (dayflags & (1<<day))   setDayTimer(day,hour,item.Temperature);
  						}
  					}
					}
				});
			  }
			 }
		  });
}

function ClearSetpointTimersInt (devIdx)
{
	$.ajax({
		 url: "json.htm?type=command&param=clearsetpointtimers&idx=" + devIdx,
		 async: false, 
		 dataType: 'json',
		 success: function(data) {
		 },
		 error: function(){
				ShowNotify($.i18n('Problem clearing timers!'), 2500, true);
		 }     
	});
}
function ProgAddSetpointTimer (devIdx,days,hour,min,val)
{
  if (typeof  val == 'undefined')	return;

  var d="";  for (var i=0;i<7;i++) if (days&(1<<i)) d = d + $.WeekDays[i] +","
  debug ("timer: "+d + " Hour:" + hour + " H Temp:" + val );

	var tsettings=GetSetpointSettings(days,hour,min,val);
	$.ajax({
		 url: "json.htm?type=command&param=addsetpointtimer&idx=" + devIdx + 
					"&active=" + tsettings.Active + 
					"&timertype=" + tsettings.timertype +
					"&date=" + tsettings.date +
					"&hour=" + tsettings.hour +
					"&min=" + tsettings.min +
					"&tvalue=" + tsettings.tvalue +
					"&days=" + tsettings.days,
		 async: false, 
		 dataType: 'json',
		 success: function(data) {
		 },
		 error: function(){
				ShowNotify($.i18n('Problem addsetpointtimer timers!'), 2500, true);
		 }     
	});
}
function ProgAdd(devIdx) {
	ClearSetpointTimersInt(devIdx);
	var lastTemp = [];
	for (var hour = 0; hour < 24; hour++) {
		var tdays = 0;
		var lastHourTemp = DayTimer[0][hour];
		var DayHourTemp = 0;
		for (day = 0; day < 7; day++) {
			var temp = DayTimer[day][hour];
			if (temp != lastTemp[day]) {
				if (lastHourTemp != temp) {
					ProgAddSetpointTimer(devIdx, tdays, hour, 0, lastHourTemp);
					tdays = 0;
					lastHourTemp = temp;
				}
				tdays |= (1 << day);
				lastTemp[day] = temp;
				DayHourTemp = temp;
			}
		}
		if (tdays != 0)
			ProgAddSetpointTimer(devIdx, tdays, hour, 0, DayHourTemp);
	}
	//            $.each($("button.btn-timer."+entry), function(i,item) {CreateTimer(devIdx,day,i,item);	});        
	ShowNotify($.i18n('Sensor Timer added!'), 2500, true);
}

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

function SetConforBkgd(obj){
//	obj.removeClass("btn-info");
//	obj.addClass("btn-confor");
	obj.addClass("btn-info");
}
function SetEcoBkgd(obj){
//	obj.addClass("btn-info");
//	obj.removeClass("btn-confor");
	obj.removeClass("btn-info");

}
function SetNoneBkgd(obj){
	obj.removeClass("btn-info");
	obj.removeClass("btn-confor");
}

function SetDayHourTemp( day,hour,confortTemp,ecoTemp)
{
	var obj = getItem(day,hour);
	if ( GetConforActivated() ){
		SetConforBkgd(obj);
		obj.html(confortTemp);
		setDayTimer(day,hour,confortTemp)
	}
	else{
		SetEcoBkgd(obj);
		obj.html(ecoTemp);
		setDayTimer(day,hour,ecoTemp)
	}
}

function SetHoursTemp( dayN,hour,confortTemp,ecoTemp)
{ 
	if (istSelectedDay()){
         SetDayHourTemp( dayN,hour,confortTemp,ecoTemp);
	}
	else{
	//selection using row and day	checkbox
     for (var day=0;day<7;day++) {
        var entry = $.WeekDays[day];
        if ( getDayCheckBox(entry).is(":checked")) 
        {
            SetDayHourTemp( day,hour,confortTemp,ecoTemp);
		}
     };
	}
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

function SetConf(obj)
{ 
  SetConfTemp($(obj).html())  ;
}

function SetConfTemp(temp)
{ 
  SetVal("#utilitycontent #tConf",temp)     ;
  selectConfButton();
}
function SetEco(obj)
{ 
  SetEcoTemp($(obj).html())  ;    ;
}
function SetEcoTemp(temp)
{ 
  SetVal("#utilitycontent #tEco" ,temp)     ;
  selectEcoButton();
}



function debug(log_txt) {
    if (typeof window.console != 'undefined') {
        console.log("DEBUG: " + log_txt);
}
}
function DisplayTimerValues(ConforT)
{
     for (day=0;day<7;day++) {
		var temp=0;
        for ( var hour = 0; hour < 24; hour++ ) {
          var obj = getItem(day,hour);
          value=DayTimer[day][hour];
		  if (typeof  value == 'undefined'){
			value = temp;
			$(obj).html('') ; 
		  }
		  else{
			temp=value;
			$(obj).html(value) ; 
		  }
          if (value>=ConforT ){
            SetConforBkgd(obj);
          }
          else{
            SetEcoBkgd(obj)
          }
		}
     };
}
//entry = Mon Tue... : read checkbox Confor
function getDayCheckBox(entry)
{
  return $("#utilitycontent #"+entry+" #Chk");
}
//day: 0..6
function getItem(day,hour)
{
  var entry = $.WeekDays[day];
  var obj = $("#utilitycontent #"+entry+" #"+hour);
  return obj;
}
function getConforVal()
{
  return $("#utilitycontent #tConf").val() ;
}
function getEcoVal()
{
  return $("#utilitycontent #tEco").val() ;
}
function GetConforActivated()
{
//    return $('#utilitycontent #ConfCkb').prop('checked');
return $.IsConfor;
}
function clearDayTimer()
{
	 for (var i=0;i<7;i++) DayTimer[i]=[];
}

function istSelectedDay()
{
  return $("#utilitycontent #when4").prop('checked');
}

function fillDayTimer()
{
/*  for (day=0;day<7;day++) {
        var value  ;  
        for ( var hour = 0; hour < 24; hour++ ) {
          var v = DayTimer[day][hour];
          if (typeof  v == 'undefined')
            DayTimer[day][hour] = value;
          else
            value = DayTimer[day][hour] ;
        }
  }*/
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

function ShowIhmSetpointTimersInt(devIdx,name, isdimmer, stype,devsubtype)
{
  $.MouseDown = false ;
  $.DayDeb = 0 ;
  $.DayEnd = 0 ;
  $.HourDeb = 0 ;
  $.HourEnd = 0 ;
  $.IsConfor = true ;
	

  SetEcoTemp(16)     ;
  SetConfTemp(20)     ;
  $.WeekDays = ["Mon","Tue","Wed","Thu","Fri","Sat","Sun"]; 
  DayTimer  = new Array(7); 
  clearDayTimer() ;
  getTimerSetPoints(devIdx) ;
  DisplayTimerValues(20);
  
  $("button.btn-timer").mouseover(function () {
     if ($.MouseDown){
        SetHoursTemp( $(this).prop("name"),$(this).prop("id"), getConforVal() , getEcoVal() );  
     }
  });
  
  $("button.btn-timer").mousedown(function () {
    $.MouseDown = true ;
	$.DayDeb = $(this).prop("name") ;
	$.HourDeb = $(this).prop("id") ;
    SetHoursTemp($.DayDeb,$.HourDeb , getConforVal() , getEcoVal() );  
  });
  
  $("button.btn-timer").mouseup(function () {
    $.MouseDown = false ;
	$.DayEnd = $(this).prop("name") ;
	$.HourEnd = $(this).prop("id") ;
	if (istSelectedDay()){
		for (var day=$.DayDeb;day<=$.DayEnd;day++)
			for (var hour=$.HourDeb;hour<=$.HourEnd;hour++)
				SetDayHourTemp( day,hour,getConforVal() , getEcoVal());
	}
  });
  
  $(window).mouseup(function(){
    $.MouseDown = false ;
  });
  $("#utilitycontent #when1").click(function() {
  	SetDays("Everyday",false);                    
  });                                                              
  $("#utilitycontent #when2").click(function() {
  	SetDays("Weekdays",false);                    
  });                                                              
  $("#utilitycontent #when3").click(function() {
  	SetDays("Weekends",false);                    
  });                                                              
  $("#utilitycontent #when4").click(function() {
  	SetDays("",false);                           
  });         

   $("#utilitycontent #tConf").click(function() {
    selectConfButton();
  });                                                              
   $("#utilitycontent #tEco").click(function() {
   selectEcoButton();
  });                                                              
  if ( $.myglobals.tempsign !='C' ){
    //conversion farenheit
    var Far=[0,59,61,62,64,66,68,70,71,73,75,77];
    for (var t=1;t<=11;t++) $('#utilitycontent #cT'+t).html(Far[t] );
    for (var t=1;t<=11;t++) $('#utilitycontent #eT'+t).html(Far[t] );
  }

}


function selectConfButton()
{
   $.each($("button.btn-conf"), function(i,item) {   $(item).addClass("btn-info");   });        
   $.each($("button.btn-eco") , function(i,item) {   $(item).removeClass("btn-info");   });       
//   $('#utilitycontent #ConfCkb').prop('checked',true);
//   $('#utilitycontent #EcoCkb').prop('checked',false);
	$.IsConfor=true;    
}
function selectEcoButton()
{
   $.each($("button.btn-conf"), function(i,item) {   $(item).removeClass("btn-info");   });        
   $.each($("button.btn-eco") , function(i,item) {   $(item).addClass("btn-info");   });        
//	$('#utilitycontent #ConfCkb').prop('checked',false);
//	$('#utilitycontent #EcoCkb').prop('checked',true);
	$.IsConfor=false;    

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
function ProgCopy(idx)
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
						  $( this ).dialog( "close" );
						  
							var SensorIdx =$("#dialog-copy #sensor option:selected").val();
							var SensorName=$("#dialog-copy #sensor option:selected").text();
							if (typeof SensorName == 'undefined') {
								bootbox.alert($.i18n('No Sensor Type Selected!'));
								return ;
							}
							clearDayTimer() ;
							clearDisplay();
							getTimerSetPoints(SensorIdx) ;
							DisplayTimerValues(20);

							bootbox.alert($.i18n('Sensor Timer '+ SensorName +' copied!'));

					  },
					  Cancel: function() {
						  $( this ).dialog( "close" );
					  }
				  },
				  close: function() {
					$( this ).dialog( "close" );
				  }
			});
			RefreshDeviceCombo("#dialog-copy #sensor" ,"utility",true) ;
			$( "#dialog-copy" ).i18n();
			$( "#dialog-copy" ).dialog( "open" );
}