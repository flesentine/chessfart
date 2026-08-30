@echo off
rem Chess Fart Build 1 - Open Watcom DOS build wrapper
wmake -f makefile.dos dos
if errorlevel 1 goto fail
echo.
echo CHESS FART Build 1 complete: build\dos\CHESSFRT.EXE
goto end
:fail
echo.
echo CHESS FART build failed.
exit /b 1
:end
