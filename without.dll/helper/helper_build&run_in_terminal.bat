@echo off
call E:\__Sowicm\_Tools\MyContextMenu\helper\helper_build %1
if %errorlevel% == 0 (
    rem call %~n1
    start "E:\__Sowicm\_Tools\MyContextMenu\helper\helper_run_in_terminal.bat" %~n1
)