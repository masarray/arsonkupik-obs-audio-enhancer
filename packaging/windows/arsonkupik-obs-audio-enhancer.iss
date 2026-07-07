; ArSonKuPik OBS Audio Enhancer installer
#define MyAppName "ArSonKuPik OBS Audio Enhancer"
#ifndef MyAppVersion
  #define MyAppVersion "0.0.0"
#endif
#ifndef SourcePackageDir
  #define SourcePackageDir "."
#endif

[Setup]
AppId={{9A160E60-EC40-4F23-AE49-7B64F51470FD}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher=masarray
DefaultDirName={code:GetDefaultPluginDir}
AppendDefaultDirName=no
DisableDirPage=yes
DirExistsWarning=no
DisableProgramGroupPage=yes
OutputDir=output
OutputBaseFilename=ArSonKuPik-OBS-Audio-Enhancer-Setup-v{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
SetupLogging=yes
CloseApplications=yes
CloseApplicationsFilter=obs64.exe,obs32.exe
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[InstallDelete]
Type: filesandordirs; Name: "{app}\bin"
Type: filesandordirs; Name: "{app}\data"

[Files]
Source: "{#SourcePackageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Run]
Filename: "https://github.com/masarray/arsonkupik-obs-audio-enhancer/releases"; Description: "View release notes"; Flags: postinstall shellexec skipifsilent unchecked

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Code]
function IsObsBinaryHere(Path: String): Boolean;
begin
  Result := FileExists(AddBackslash(Path) + 'bin\64bit\obs64.exe') or FileExists(AddBackslash(Path) + 'bin\64-bit\obs64.exe') or FileExists(AddBackslash(Path) + 'bin\obs64.exe');
end;

function FindObsInstallDir(): String;
var
  InstallDir: String;
begin
  Result := '';

  if RegQueryStringValue(HKLM64, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OBS Studio_is1', 'InstallLocation', InstallDir) then
  begin
    if DirExists(InstallDir) then
    begin
      Result := InstallDir;
      exit;
    end;
  end;

  if RegQueryStringValue(HKCU, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OBS Studio_is1', 'InstallLocation', InstallDir) then
  begin
    if DirExists(InstallDir) then
    begin
      Result := InstallDir;
      exit;
    end;
  end;

  InstallDir := ExpandConstant('{pf}\obs-studio');
  if IsObsBinaryHere(InstallDir) then
  begin
    Result := InstallDir;
    exit;
  end;

  InstallDir := ExpandConstant('{pf32}\obs-studio');
  if IsObsBinaryHere(InstallDir) then
  begin
    Result := InstallDir;
    exit;
  end;
end;

function GetDefaultPluginDir(Default: String): String;
begin
  Result := ExpandConstant('{commonappdata}\obs-studio\plugins\arsonkupik-obs-audio-enhancer');
end;

function InitializeSetup(): Boolean;
var
  ObsDir: String;
begin
  Result := True;
  ObsDir := FindObsInstallDir();

  if ObsDir = '' then
  begin
    MsgBox('OBS Studio was not detected in the usual locations. The installer will still install the plugin into the standard OBS ProgramData plugin folder:' + #13#10 + #13#10 + ExpandConstant('{commonappdata}\obs-studio\plugins\arsonkupik-obs-audio-enhancer') + #13#10 + #13#10 + 'Install OBS Studio first if you have not installed it yet.', mbInformation, MB_OK);
  end;
end;
