*****************************************************************
***                 Locate NSIS plugin v2.0                   ***
*****************************************************************

2011 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)


Features:
1. Search for directories or empty directories and/or files
   -search with wildcard
   -search with names filter (include/exclude)
   -search with extensions filter (include/exclude)
   -search with path filter
   -search with times filter
   -search with size filter
   -search with attributes filter
   -sort files and/or directories by name, type, size, date (with reverse or not)
   -output first files or directories
   -search with subdirectories or not
   -search with banner support
   -return path\name
   -return path
   -return name
   -return size of file
   -return date of file or directory
   -return attributes of file or directory
2. Find the size of a file, files wildcard or directory (locate::GetSize)
3. Find the sum of the files, directories and subdirectories (locate::GetSize)
4. Remove empty directories and/or subdirectories (locate::RMDirEmpty)


**** Open for search ****

${locate::Open} "[Paths]" "[Options]" $var

"[Paths]"     - Paths to search in ("D:\Temp", "D:\Temp|C:\WINDOWS", ...)

"[Options]"   - Search options

	/F=[1|0]
		/F=1  - Locate Files (default)
		/F=0  - Don't locate Files
	/D=[1|0]
		/D=1  - Locate Directories (default)
		/D=0  - Don't locate Directories
	/DE=[0|1]
		/DE=0  - Don't locate Empty Directories (default)
		/DE=1  - Locate Empty Directories (if /DE=1 then /D=0 will be set)
	/M=[wildcard]
		/M=*.*          - Locate all (default)
		/M=*.doc        - Locate Work.doc, 1.doc ...
		/M=Pho*         - Locate PHOTOS, phone.txt ...
		/M=win???.exe   - Locate winamp.exe, winver.exe ...
		/M=winamp.exe   - Locate winamp.exe only
	/N=[names]
		                         - Locate all (default)
		/N=readme.txt            - Locate only "readme.txt"
		/N=Setup.exe|soft        - Locate only "Setup.exe", "soft"
	/-N=[names]
		/-N=readme.txt           - Don't locate "readme.txt"
		/-N=Setup.exe|soft       - Don't locate "Setup.exe", "soft"
	/X=[extensions]
		                         - Locate all (default)
		/X=                      - Locate only files without extension
		/X=exe|com|bat           - Locate only files *.exe, *.com, *.bat
	/-X=[extensions]
		/-X=                     - Don't locate files without extension
		/-X=exe|com|bat          - Don't locate files *.exe, *.com, *.bat
	/-PF=[paths]
		                         - Locate all (default)
		/-PF=E:\Incoming         - Don't locate in "E:\Incoming" path
		/-PF=D:\Temp|C:\WINDOWS  - Don't locate in "D:\Temp", "C:\WINDOWS" paths
	/-PN=[directories]
		                         - Locate all (default)
		/-PN=Incoming            - Don't locate in "Incoming" directory
		/-PN=Temp|WINDOWS        - Don't locate in "Temp", "WINDOWS" directories
	/T=day.month.year:day.month.year,[Write|Creation|Access]
		/T=,Write                          - Locate all and Write time to output (default)
		/T=29.08.2005:29.08.2005,Creation  - Locate files and directories created 29.08.2005
		/T=01.08.2005:29.08.2005,Creation  - Locate files and directories created between 01.08.2005 and 29.08.2005
		/T=:28.08.2005,Creation            - Locate files and directories created before 28.08.2005
		/T=01.12.1999:,Creation            - Locate files and directories created after 01.12.1999
	/S=more:less,[Bytes|Kb|Mb|Gb]
		/S=,Bytes        - Locate all and files size in Bytes to output (default)
		/S=0:0,Bytes     - Locate only files of 0 Bytes exactly
		/S=5:9,Kb        - Locate only files of 5 to 9 Kilobytes
		/S=:10,Mb        - Locate only files of 10 Megabyte or less
		/S=1:,Gb         - Locate only files of 1 Gigabyte or more
	/A=[READONLY|ARCHIVE|HIDDEN|SYSTEM|-READONLY|-ARCHIVE|-HIDDEN|-SYSTEM]
		                                        - Locate all (default)
		/A=READONLY                             - Locate files and directories with attribute READONLY (e.g. "ra--")
		/A=READONLY|SYSTEM                      - Locate files and directories with attribute READONLY and SYSTEM (e.g. "r-hs")
		/A=READONLY|-SYSTEM                     - Locate files and directories with attribute READONLY and without SYSTEM (e.g. "r-h-")
		/A=READONLY|ARCHIVE|HIDDEN|SYSTEM       - Locate files and directories with all attribute ("rahs")
		/A=-READONLY|-ARCHIVE|-HIDDEN|-SYSTEM   - Locate files and directories with attribute NORMAL ("----")
	/SF=[NAME|TYPE|SIZE|DATE]
		          - Don't sort files (default)
		/SF=NAME  - Sort Files by name
		/SF=TYPE  - Sort Files by type
		/SF=SIZE  - Sort Files by syze
		/SF=DATE  - Sort Files by date
	/-SF=[NAME|TYPE|SIZE|DATE]
		/-SF=NAME  - Sort Files by name reverse order
		/-SF=TYPE  - Sort Files by type reverse order
		/-SF=SIZE  - Sort Files by syze reverse order
		/-SF=DATE  - Sort Files by date reverse order
	/SD=[NAME|DATE]
		          - Don't sort directories (default)
		/SD=NAME  - Sort Directories by name
		/SD=DATE  - Sort Directories by date
	/-SD=[NAME|DATE]
		/-SD=NAME  - Sort Directories by name reverse order
		/-SD=DATE  - Sort Directories by date reverse order
	/R=[0|1]
		/R=0       - Don't Reverse output - first Directories next Files (default)
		/R=1       - Reverse output - first Files next Directories
	/G=[1|0]
		/G=1       - Locate with subdirectories (default)
		/G=0       - Don't locate subdirectories
	/B=[0|1|2]
		/B=0       - Don't show searching path (default)
		/B=1       - Show in the details current searching path
		/B=2       - Banner is used. Return: "path", "", "", "", "", ""
		             when starts to search in new directory

$var     Handle, zero if error


**** Find first and next (call one or more times) ****

${locate::Find} "[handle]" $var1 $var2 $var3 $var4 $var5 $var6

"[handle]"   handle returned by locate::Open

$var1        "path\name"
$var2        "path"
$var3        "name"
$var4        "size"        ("" if directory founded)
$var5        "time"        (e.g. "29.08.2005 14:27:18")
$var6        "attributes"  (e.g. "r-h-", "rahs")


**** Close search (free memory) ****

${locate::Close} "[handle]"

"[handle]"   handle returned by locate::Open


**** GetSize ****

${locate::GetSize} "[Path]" "[Options]" $var1 $var2 $var3

"[Path]"      - Disk or Directory

"[Options]"   - Search options

	/S=[Bytes|Kb|Mb|Gb]
		/S=Bytes       - Return size in bytes (default)
		/S=Kb          - Return size in kilobytes
		/S=Mb          - Return size in megabytes
		/S=Gb          - Return size in gigabytes
	/M=[mask]
		/M=*.*         - Find all (default)
		/M=*.doc       - Find Work.doc, 1.doc ...
		/M=Pho*        - Find PHOTO.JPG, phone.txt ...
		/M=win???.exe  - Find winamp.exe, winver.exe ...
		/M=winamp.exe  - Find winamp.exe only
	/G=[1|0]
		/G=1           - Find with subdirectories (default)
		/G=0           - Find without subdirectories
	/B=[0|1]
		/B=0           - Don't show searching path (default)
		/B=1           - Show in the details current searching path

$var1  - Size                ("-1" if error)
$var2  - Sum of files        ("-1" if error)
$var3  - Sum of directories  ("-1" if error)


**** Remove empty directories ****

${locate::RMDirEmpty} "[Path]" "[Options]" $var

"[Path]"      - Disk or Directory

"[Options]"   - Search options

	/M=[mask]
		/M=*.*         - Remove all (default)
		/M=*th         - Remove Math, LAUGH ...
		/M=Pho*        - Remove PHOTOS, phones ...
		/M=win???      - Remove winamp, WinRAR ...
	/G=[1|0]
		/G=1           - Remove in subdirectories (default)
		/G=0           - Don't remove in subdirectories
	/B=[0|1]
		/B=0           - Don't show searching path (default)
		/B=1           - Show in the details current searching path

$var  - Sum of removed directories   ("-1" if error)


**** Unload plugin ****

${locate::Unload}
