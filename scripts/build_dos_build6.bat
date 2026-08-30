@echo off
wmake -f makefile.build6.dos clean
if errorlevel 1 exit /b 1
wmake -f makefile.build6.dos dos
