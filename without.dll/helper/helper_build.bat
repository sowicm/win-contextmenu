@echo off
setlocal enabledelayedexpansion

if exist %cd%\%~n1.exe del %cd%\%~n1.exe

cl %1 /G7 /GA /GL /O2 /Og /Ot /Ox /W3 /EHsc /I"E:\__Sowicm\_Projects\_Lib\include" /I"E:\__Sowicm\_Projects\_Lib\include\modules"  /link /LIBPATH:"E:\__Sowicm\_Projects\_Lib\lib"

rem del %cd%\%~n1.obj

if not %errorlevel% == 0 pause
