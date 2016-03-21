@echo off

rem 文件夹背景啥的也修改修改

echo 定制右键
pause

set p1=%%1
set file=%%p1%%

rem class.*
set sprefix=HKEY_CLASSES_ROOT\*\shell

call :mainmenu

call :additem "Run in terminal" "E:\__Sowicm\_Tools\MyContextMenu\helper\helper_run_in_terminal \"%file%\""
call :additem "Open with..." "*"

rem class.cpp
set sprefix=HKEY_CLASSES_ROOT\cpp_auto_file\shell

call :mainmenu

call :additem "Build" "E:\__Sowicm\_Tools\MyContextMenu\helper\helper_build \"%file%\""
call :additem "Build&&Run" "E:\__Sowicm\_Tools\MyContextMenu\helper\helper_build&run \"%file%\""
call :additem "Build&&Run in terminal" "E:\__Sowicm\_Tools\MyContextMenu\helper\helper_build&run_in_terminal \"%file%\""


rem class.folder
set sprefix=HKEY_CLASSES_ROOT\Folder\shell

call :mainmenu

call :additem "Clear desktop.ini" "E:\__Sowicm\_Tools\MyContextMenu\helper\helper_desktop \"%file%\""

rem directory.backgroud
set sprefix=HKEY_CLASSES_ROOT\Directory\Background\shell

call :mainmenu

call :additem "Open Terminal" "cmd"
call :additem "Open Terminal as Admin" "runas /noprofile /env /user:admin cmd"

call :additem "Clear R/S/H Attributes" "E:\__Sowicm\_Tools\MyContextMenu\helper\helper_clear_rsh_attrib"

goto end

:mainmenu
reg delete "%sprefix%\sowicm" /f
reg add "%sprefix%\sowicm" /f
reg delete "%sprefix%\sowicm" /ve /f

reg add "%sprefix%\sowicm" /v "MUIVerb" /t REG_SZ /d "颤抖吧！凡人！" /f
reg add "%sprefix%\sowicm" /v "subcommands" /t REG_SZ /d "" /f
set /a nItems = 0

call :additem "颤抖吧！凡人！" "explorer \"http://weibo.com/zhizunmingshuai\"" 0x40

goto :eof

:additem
rem %1 is name, %2 is script [%3 is CommandFlags]
set arg3=%3
set /a nItems += 1
reg add "%sprefix%\sowicm\shell\s_cmd%nItems%" /v "MUIVerb" /t REG_SZ /d %1 /f
reg add "%sprefix%\sowicm\shell\s_cmd%nItems%\command" /ve /t REG_SZ /d %2 /f
if defined arg3 reg add "%sprefix%\sowicm\shell\s_cmd%nItems%" /v "CommandFlags" /t REG_DWORD /d %3 /f
goto :eof

:end
pause
