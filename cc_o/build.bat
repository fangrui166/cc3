@echo off
echo *
echo *
echo ******** Start build CC-O ********
echo *
echo *
set pwd=%~dp0
set bootloader_size=16384

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
echo %pwd%
(for /f "tokens=*" %%i in (%pwd%application\Inc\version.h) do echo %%i|findstr /i /c:"MAINIMAGE_VERSION_STRING">nul&&echo %sys_version%||echo %%i)>temp.h
move /y temp.h %pwd%application\Inc\version.h
(for /f "tokens=*" %%i in (%pwd%application\Inc\version.h) do echo %%i|findstr /i /c:"BOOTLOADER_VERSION_STRING">nul&&echo %bl_version%||echo %%i)>temp.h
move /y temp.h %pwd%application\Inc\version.h

:build
IarBuild %pwd%application\EWARM\cc_o_app.ewp -build Release -log all
echo %errorlevel%
if %errorlevel% EQU 0 (
	if not exist %pwd%..\out\eng\cc_o\ md %pwd%..\out\eng\cc_o\
	if not exist %pwd%..\out\web\cc_o\ md %pwd%..\out\web\cc_o\
	copy %pwd%application\EWARM\Release\Exe\cc_o_app.bin %pwd%..\out\eng\cc_o\ 
	%pwd%..\tools\add_crc %pwd%..\out\eng\cc_o\cc_o_app.bin
	move /Y %pwd%..\out\eng\cc_o\cc_o_app.bin.crc %pwd%..\out\web\cc_o\
) else (
	set ret_error=1
	goto :done
)
IarBuild %pwd%bootloader\EWARM\cc_o_bl.ewp -build Release -log all
echo %errorlevel%
if %errorlevel% EQU 0 (
	if not exist %pwd%..\out\eng\cc_o\ md %pwd%..\out\eng\cc_o\
	if not exist %pwd%..\out\web\cc_o\ md %pwd%..\out\web\cc_o\
	copy %pwd%bootloader\EWARM\Release\Exe\cc_o_bl.bin %pwd%..\out\eng\cc_o\ 
	%pwd%..\tools\add_crc %pwd%..\out\eng\cc_o\cc_o_bl.bin
	move %pwd%..\out\eng\cc_o\cc_o_bl.bin.crc %pwd%..\out\web\cc_o\

	if exist %pwd%..\out\eng\cc_o\cc_o_app.bin (
		if exist %pwd%..\out\eng\cc_o\factory_release.bin del %pwd%..\out\eng\cc_o\factory_release.bin
		%pwd%..\tools\create %pwd%..\out\eng\cc_o\factory_release.bin %bootloader_size%
		%pwd%..\tools\cat %pwd%..\out\eng\cc_o\cc_o_app.bin>>%pwd%..\out\eng\cc_o\factory_release.bin
		%pwd%..\tools\hbin %pwd%..\out\eng\cc_o\cc_o_bl.bin %pwd%..\out\eng\cc_o\factory_release.bin
	)
) else (
	set ret_error=1
	goto :done
)
set ret_error=0
:done