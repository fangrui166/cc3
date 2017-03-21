@echo off
rm -rf ./out
set ret_error=0

call ./cc_i/build.bat %1 %2
echo return %ret_error% 
if %ret_error% EQU 1 (
	goto :Done
)
call ./cc_c/build.bat %1 %2
if %ret_error% EQU 1 (
	goto :Done
)
call ./cc_o/build.bat %1 %2
if %ret_error% EQU 1 (
	goto :Done
)

:Done
pause