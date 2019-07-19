# Building for Windows, on Windows

Download ["qt-opensource-windows-x86-mingw492-5.6.0.exe"](https://download.qt.io/archive/qt/5.6/5.6.0/).

Download [CMake](https://cmake.org/download/).

Clone the repo to `C:\src\brewtarget`.

Launch a command shell using the **Qt 5.6 for Desktop (MinGW 4.9.2 32 bit)** shortcut and run the following:

```
cd c:\src\brewtarget-build
cmake -G "MinGW Makefiles" -DDO_RELEASE_BUILD=ON -DCMAKE_PREFIX_PATH="C:\Qt\5.6\mingw49_32" ../brewtarget
C:\Qt\5.6\Tools\mingw492_32\bin\mingw32-make.exe
mkdir data
copy c:\src\brewtarget\data c:\src\brewtarget-build\data
copy C:\Qt\5.6\5.6\mingw49_32\bin\libgcc_s_dw2-1.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\libwinpthread-1.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Core.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Gui.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Multimedia.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5MultimediaWidgets.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Network.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5OpenGL.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Positioning.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5PrintSupport.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Qml.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Quick.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Sensors.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Sql.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Svg.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Widgets.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Xml.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Guid.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Networkd.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Multimediad.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Cored.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5PrintSupportd.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Sqld.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Widgetsd.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\bin\Qt5Xmld.dll C:\src\brewtarget-build\src
copy "C:\Qt\5.6\5.6\mingw49_32\bin\libstdc++-6.dll" C:\src\brewtarget-build\src
echo iconengines
copy C:\Qt\5.6\5.6\mingw49_32\plugins\iconengines\qsvgicon.dll C:\src\brewtarget-build\src
copy C:\Qt\5.6\5.6\mingw49_32\plugins\iconengines\qsvgicond.dll C:\src\brewtarget-build\src
echo imageformats
copy C:\Qt\5.6\5.6\mingw49_32\plugins\imageformats\ C:\src\brewtarget-build\src

```

Create a desktop shortcut:

  * Location: `C:\src\brewtarget-build\src\brewtarget.exe`
  * Name: `Brewtarget devel`

Pieced together from:

* https://github.com/Brewtarget/brewtarget/issues/397#issuecomment-375105783
* https://github.com/Brewtarget/brewtarget/blob/develop/dev-doc/WindowsPackaging.txt
