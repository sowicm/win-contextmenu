@echo off
setlocal enabledelayedexpansion

set s=%1\desktop.ini
choice /M "are you sure del %s%"
if errorlevel 2 exit
del /s /ah %s%
pause
