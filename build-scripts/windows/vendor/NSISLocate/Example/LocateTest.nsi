Name "LocateTest"
OutFile "LocateTest.exe"

!include "Locate.nsh"
!include "Sections.nsh"

Var RADIOBUTTON

Page components
Page instfiles


Section "Search for text files" SearchTxt
	${locate::Open} "$WINDIR" `/F=1 /D=0 /M=*.txt /B=1` $0
	StrCmp $0 0 0 loop
	MessageBox MB_OK "Error" IDOK close

	loop:
	${locate::Find} $0 $1 $2 $3 $4 $5 $6

	MessageBox MB_OKCANCEL '$$1    "path\name"   =[$1]$\n\
					$$2    "path"             =[$2]$\n\
					$$3    "name"           =[$3]$\n\
					$$4    "size"             =[$4]$\n\
					$$5    "time"             =[$5]$\n\
					$$6    "attributes      =[$6]$\n\
									$\n\
					Find next?' IDOK loop
	close:
	${locate::Close} $0
	${locate::Unload}
SectionEnd


Section /o "Search with filters" SearchWithFilters
	#Locate files in Windows directory beginning with "set",
	# with extensions "log" or "txt", but not "setuplog.txt", not "setuperr.log",
	# exclude "C:\WINDOWS\System", "C:\WINDOWS\System32", "C:\WINDOWS\Help" paths from search,
	# exclude "config" directory from search (C:\WINDOWS\system32\config, C:\WINDOWS\Config, D:\WINDOWS\pchealth\helpctr\Config, ...)

	${locate::Open} "$WINDIR" `/F=1 \
					/D=0 \
					/M=set* \
					/X=log|txt \
					/-N="setuplog.txt|setuperr.log" \
					/-PF="$WINDIR\System|$WINDIR\System32|$WINDIR\Help" \
					/-PN=config \
					/B=1` $0
	StrCmp $0 -1 0 loop
	MessageBox MB_OK "Error" IDOK close

	loop:
	${locate::Find} $0 $1 $2 $3 $4 $5 $6

	MessageBox MB_OKCANCEL '$$1    "path\name"   =[$1]$\n\
					$$2    "path"             =[$2]$\n\
					$$3    "name"           =[$3]$\n\
					$$4    "size"             =[$4]$\n\
					$$5    "time"             =[$5]$\n\
					$$6    "attributes      =[$6]$\n\
									$\n\
					Find next?' IDOK loop
	close:
	${locate::Close} $0
	${locate::Unload}
SectionEnd


Section /o "Search and write founded in text file" SearchAndWriteInFile
	GetTempFileName $1
	FileOpen $2 $1 w

	${locate::Open} "$PROGRAMFILES\Common Files" `/F=1 /D=1 /S=,Kb /T=,Creation /SF=TYPE /SD=NAME /B=1` $R0
	StrCmp $R0 0 0 loop
	MessageBox MB_OK "Error" IDOK close

	loop:
	${locate::Find} $R0 $R1 $R2 $R3 $R4 $R5 $R6

	StrCmp $R1 '' close
	StrCmp $R4 '' 0 +3
	FileWrite $2 'DIR:"$R1" [Created: $R5] [$R6]$\r$\n'
	goto +2
	FileWrite $2 'FILE:"$R1" [$R4 Kb] [Created: $R5] [$R6]$\r$\n'
	goto loop

	close:
	${locate::Close} $R0
	${locate::Unload}
	FileClose $2

	Exec '"notepad.exe" "$1"'
SectionEnd


### If nsx.dll not found then "Search with banner" example will not be compile ###
!system 'ECHO.>"%TEMP%\Temp$$$.nsh"'
!system 'IF EXIST "${NSISDIR}\Plugins\nxs.dll" ECHO !define nxs_exist>>"%TEMP%\Temp$$$.nsh"'
!include "$%TEMP%\Temp$$$.nsh"
!system 'DEL "%TEMP%\Temp$$$.nsh"'

!ifdef nxs_exist
Section /o "Search with banner - 'NxS' plugin required" SearchWithBanner
	HideWindow

	nxs::Show /NOUNLOAD `$(^Name) Setup`\
		/top `Setup searching something$\nPlease wait... If you can...`\
		/pos 78 /h 0 /can 1 /end

	${locate::Open} "$WINDIR" `/F=1 /D=1 /SF=TYPE /M="Unexisted Name.ext" /SD=NAME /B=2` $0
	StrCmp $0 0 0 loop
	nxs::Destroy
	MessageBox MB_OK "Error" IDOK close

	loop:
	${locate::Find} $0 $1 $2 $3 $4 $5 $6

	StrCmp $1 '' close
	StrCmp $6 '' 0 code
	nxs::Update /NOUNLOAD /sub "$1" /end
	nxs::HasUserAborted /NOUNLOAD
	Pop $1
	StrCmp $1 1 close loop

	code:
	;... code
	goto loop

	close:
	${locate::Close} $0
	${locate::Unload}
	nxs::Destroy

	BringToFront
SectionEnd
!endif


Section /o "Get size of the 'C:\Program Files'" GetSize
	${locate::GetSize} "C:\Program Files" `/S=Mb /M=*.* /G=1 /B=1` $R1 $R2 $R3
	${locate::Unload}
	MessageBox MB_OK "locate::GetSize$\n$\n\
			Size:     [$R1]$\n\
			Files:     [$R2]$\n\
			Folders: [$R3]"
SectionEnd


Section /o "Remove all empty directories" RMDirEmpty
	CreateDirectory "$TEMP\1\2\a\a"
	CreateDirectory "$TEMP\1\3\a\a"

	${locate::RMDirEmpty} "$TEMP\1" `/M=*.* /G=1 /B=1` $R1
	${locate::Unload}
	MessageBox MB_OK "locate::RMDirEmpty$\n$\n\
			Removed Directories: [$R1]"

	RMDir "$TEMP\1"
SectionEnd


Function .onInit
	StrCpy $RADIOBUTTON ${SearchTxt}
FunctionEnd

Function .onSelChange
	!insertmacro StartRadioButtons $RADIOBUTTON
	!insertmacro RadioButton ${SearchTxt}
	!insertmacro RadioButton ${SearchWithFilters}
	!insertmacro RadioButton ${SearchAndWriteInFile}
	!ifdef nxs_exist
		!insertmacro RadioButton ${SearchWithBanner}
	!endif
	!insertmacro RadioButton ${GetSize}
	!insertmacro RadioButton ${RMDirEmpty}
	!insertmacro EndRadioButtons
FunctionEnd

