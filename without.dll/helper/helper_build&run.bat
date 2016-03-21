@echo off
call E:\__Sowicm\_Tools\MyContextMenu\helper\helper_build %1
if %errorlevel% == 0 (
    start %~n1
)