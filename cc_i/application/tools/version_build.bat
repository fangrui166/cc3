@echo off
if "%1"=="" goto :build
if "%2"=="" goto :build
set sys=%1
set bl=%2
set "sys_pre=#define MAINIMAGE_VERSION_STRING"
set "bl_pre=#define BOOTLOADER_VERSION_STRING"
set sys_version=%sys_pre% %sys%
set bl_version=%bl_pre% %bl%
echo %sys_version%
echo %bl_version%
(for /f "tokens=*" %%i in (..\Inc\version.h) do echo %%i|findstr /i /c:"MAINIMAGE_VERSION_STRING">nul&&echo %sys_version%||echo %%i)>temp.h
move /y temp.h ..\Inc\version.h
(for /f "tokens=*" %%i in (..\Inc\version.h) do echo %%i|findstr /i /c:"BOOTLOADER_VERSION_STRING">nul&&echo %bl_version%||echo %%i)>temp.h
move /y temp.h ..\Inc\version.h
:build
IarBuild ..\EWARM\cc_i.ewp -clean Debug -log all
pause
