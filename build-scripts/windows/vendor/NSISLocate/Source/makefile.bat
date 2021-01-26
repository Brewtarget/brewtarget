@ECHO OFF
Set MINGW=C:\MinGW
Set BIT=32

::#######################::
Set PATH=%MINGW%\bin;%PATH%

mingw32-make.exe BIT=%BIT% all clean

if not "%1" == "/S" PAUSE
CLS
