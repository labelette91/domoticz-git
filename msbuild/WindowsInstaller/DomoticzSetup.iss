; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Domoticz"
#define AppId "{{EC4A5746-2655-43CD-AC5F-73F4B2C12F46}"
#define MyAppPublisher "Domoticz.com"
#define MyAppURL "http://www.domoticz.com/"
#define MyAppExeName "domoticz.exe"
#define NSSM "nssm.exe"
#define SetupBaseName   "DomoticzSetup_"
#define SetupName   "DomoticzSetup"
#dim Version[4]
#expr ParseVersion("..\Release\domoticz.exe", Version[0], Version[1], Version[2], Version[3])
#define AppVersion Str(Version[0]) + "." + Str(Version[1]) + "." + Str(Version[2]) + "." + Str(Version[3])
#define ShortAppVersion Str(Version[0]) + "." + Str(Version[3])
#define ShortAppVersionUnderscore Str(Version[0]) + "_" + Str(Version[3])

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={#AppId}
AppName={#MyAppName}
AppVersion={#ShortAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=..\..\License.txt
OutputDir=.
; OutputBaseFilename={#SetupBaseName + ShortAppVersionUnderscore}
OutputBaseFilename={#SetupName}
SetupIconFile=install.ico
Compression=lzma2
PrivilegesRequired=admin
SolidCompression=yes
UsePreviousAppDir=yes
DirExistsWarning=no
WizardImageFile=compiler:WizModernImage-IS.bmp
WizardSmallImageFile=compiler:WizModernSmallImage-IS.bmp

[Tasks]
Name: RunAsApp; Description: "Run as application "; Flags: exclusive;
Name: RunAsApp\desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; 
Name: RunAsApp\quicklaunchicon; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked;
Name: RunAsApp\startupicon; Description: "Create a Startup Shortcut"; GroupDescription: "{cm:AdditionalIcons}"; 
Name: RunAsService; Description: "Run as service"; Flags: exclusive unchecked

[Files]
Source: "..\Release\domoticz.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\www\*"; DestDir: "{app}\www"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\..\Config\*"; DestDir: "{app}\Config"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\..\scripts\*"; DestDir: "{app}\scripts"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\..\dzVents\*"; DestDir: "{app}\dzVents"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\Debug\libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Windows Libraries\OpenZwave\Release\OpenZWave.dll"; DestDir: {app}; Flags: ignoreversion;
Source: "..\..\Manual\DomoticzManual.pdf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\History.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\nssm.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\server_cert.pem"; DestDir: "{app}"; Flags: onlyifdoesntexist uninsneveruninstall

[Icons]
Name: "{group}\Domoticz"; Filename: "{app}\{#MyAppExeName}"; Parameters: "{code:GetParams}" ; Tasks: RunAsApp; 
;Name: "{group}\Start Domoticz service"; Filename: "sc"; Parameters: "start {#MyAppName}"; Tasks: RunAsService; 
;Name: "{group}\Stop Domoticz service"; Filename: "sc"; Parameters: "stop {#MyAppName}"; Tasks: RunAsService; 
Name: "{group}\DomoticzManual.pdf"; Filename: "{app}\DomoticzManual.pdf"; 
Name: "{group}\{cm:ProgramOnTheWeb,Domoticz}"; Filename: "{#MyAppURL}";
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"; 
Name: "{commonstartup}\Domoticz"; Filename: "{app}\{#MyAppExeName}"; Parameters: "-startupdelay 10 {code:GetParams}" ; Tasks: RunAsApp\startupicon
Name: "{commondesktop}\Domoticz"; Filename: "{app}\{#MyAppExeName}"; Parameters: "{code:GetParams}" ; Tasks: RunAsApp\desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Domoticz"; Filename: "{app}\{#MyAppExeName}"; Tasks: RunAsApp\quicklaunchicon

[Run]
;Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Flags: nowait postinstall skipifsilent runascurrentuser; Tasks: RunAsApp
Filename: "{app}\{#NSSM}"; Parameters: "install {#MyAppName} ""{app}\{#MyAppExeName}"" ""{code:GetParamsService}"""; Flags: runhidden; Tasks: RunAsService
Filename: "{sys}\net.exe"; Parameters: "start {#MyAppName}"; Flags: runhidden; Tasks: RunAsService

[Dirs]
Name: "{app}\backups\hourly"
Name: "{app}\backups\daily"
Name: "{app}\backups\monthly"
Name: "{app}\log"; Permissions: everyone-full

[PostCompile]
Name: "S:\Domoticz\msbuild\WindowsInstaller\makedist.bat"; Flags: cmdprompt redirectoutput

[InstallDelete]
Type: filesandordirs; Name: "{app}\scripts\dzVents\documentation"
Type: filesandordirs; Name: "{app}\scripts\dzVents\runtime"


[Code]
var
  ConfigPage: TInputQueryWizardPage;
  LogConfigPage: TInputDirWizardPage;
 
function GetParams(Value: string): string;
begin
  Result := '-www '+ConfigPage.Values[0]+' -sslwww '+ConfigPage.Values[1];
end;

function GetParamsService(Value: string): string;
begin
  Result := '-www '+ConfigPage.Values[0]+' -sslwww '+ConfigPage.Values[1];
end;

procedure InitializeWizard;
begin
  // Create the page
  ConfigPage := CreateInputQueryPage(wpSelectComponents,
  'User settings', 'Port number', 'Please specify the port on which Domoticz will run');
  // Add items (False means it's not a password edit)
  ConfigPage.Add('HTTP Port number:', False);
  // Set initial values (optional)
  ConfigPage.Values[0] := ExpandConstant('8080');

  ConfigPage.Add('HTTPS Port number:', False);
  // Set initial values (optional)
  ConfigPage.Values[1] := ExpandConstant('443');

  LogConfigPage := CreateInputDirPage(wpSelectComponents,
    'Select Log File Location', 'Where should the log file be stored?',
    'The log file will be stored in the installation folder by default.'#13#10 +
    'If you do not wish to retain the log file permanently, select a temp folder'#13#10 +
    '(c:\windows\temp for instance).'#13#10 +
    'To continue, click Next. If you would like to select a different folder, click Browse.',
    False, 'New Folder');
  LogConfigPage.Add('');

  LogConfigPage.Values[0] := WizardDirValue+'\log';
 
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
begin
  if(CurStep = ssInstall) then begin
    Exec('sc',ExpandConstant('stop "{#MyAppName}"'),'', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    Exec('sc',ExpandConstant('delete "{#MyAppName}"'),'', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID=wpFinished then
  begin
   WizardForm.FinishedLabel.Caption := 'Setup has finished installing Domoticz. If you installed as a service, Domoticz will now start automatically.' + #13#10 + 'Otherwise you may start Domoticz from the start menu.'+ #13#10#13#10 + 'Click Finish to exit Setup.'; 
 end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then begin
    Exec('sc',ExpandConstant('stop "{#MyAppName}"'),'', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    Exec('sc',ExpandConstant('delete "{#MyAppName}"'),'', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    sleep(4000); //allow service to stop before deleting files
  end;
end;

function InitializeSetup: Boolean;                                    

begin
  if RegValueExists(HKEY_LOCAL_MACHINE,'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#emit StringChange(SetupSetting("AppId"),"{{","{")}_is1', 'UninstallString') then begin
    MsgBox('You are upgrading an existing installation of Domoticz.'+ chr(13) +'It is recommended to reboot your system after this upgrade'+ chr(13) +'in order to avoid com port issues.', mbInformation, MB_OK);
  end;
  Result := True;
end;
