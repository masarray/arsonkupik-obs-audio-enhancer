#ifndef MyAppVersion
#define MyAppVersion "dev"
#endif

#define MyAppName "ArSonKuPik OBS Audio Enhancer"
#define MyAppPublisher "Tutorial Mas Ari / ArSonKuPik Contributors"
#define MyAppURL "https://github.com/masarray/arsonkupik-obs-audio-enhancer"
#define MyPluginName "arsonkupik-obs-audio-enhancer"
#define MyPluginDll "arsonkupik-obs-audio-enhancer.dll"

[Setup]
AppId={{5DDFD208-1C3A-4307-B5AE-0E9BE64CE3BE}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={commonappdata}\obs-studio\plugins\{#MyPluginName}
DefaultGroupName={#MyAppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=..\..\dist
OutputBaseFilename=ArSonKuPik-OBS-Audio-Enhancer-Setup-{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
UninstallDisplayName={#MyAppName}
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription=Native OBS Studio smart audio enhancer filter
VersionInfoProductName={#MyAppName}
SetupLogging=yes
CloseApplications=yes
CloseApplicationsFilter=obs64.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\..\package-programdata\{#MyPluginName}\bin\64bit\{#MyPluginDll}"; DestDir: "{commonappdata}\obs-studio\plugins\{#MyPluginName}\bin\64bit"; Flags: ignoreversion
Source: "..\..\package-programdata\{#MyPluginName}\data\locale\en-US.ini"; DestDir: "{commonappdata}\obs-studio\plugins\{#MyPluginName}\data\locale"; Flags: ignoreversion
Source: "..\..\release\README_INSTALL.txt"; DestDir: "{commonappdata}\obs-studio\plugins\{#MyPluginName}"; Flags: ignoreversion isreadme

[Icons]
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

[Run]
Filename: "notepad.exe"; Parameters: "{commonappdata}\obs-studio\plugins\{#MyPluginName}\README_INSTALL.txt"; Description: "Open install notes"; Flags: postinstall skipifsilent nowait

[UninstallDelete]
Type: filesandordirs; Name: "{commonappdata}\obs-studio\plugins\{#MyPluginName}"

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
  if MsgBox('Please close OBS Studio before installing ArSonKuPik OBS Audio Enhancer.' #13#10 #13#10 +
            'Continue installation?', mbConfirmation, MB_YESNO) = IDNO then
  begin
    Result := False;
  end;
end;
