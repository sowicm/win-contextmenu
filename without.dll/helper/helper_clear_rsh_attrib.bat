@echo off
set/p confirm=Are you sure?(y/n)
if %confirm% == y (attrib -r -s -h * /S /D /L) else (pause)