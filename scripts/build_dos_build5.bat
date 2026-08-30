@echo off
wmake -f makefile.build5.dos dos
if errorlevel 1 exit /b 1
echo.
echo Build 5 DOS target complete.
