# Microsoft Developer Studio Project File - Name="Puzilla" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO EDIT **
# TARGTYPE "Win32 (x86) Console Application" 0x0103
# TARGTYPE "Win32 (x86) External Target" 0x0106
CFG=Puzilla - Win32 Debug
!MESSAGE NMAKE /f "Puzilla.mak".
!MESSAGE NMAKE /f "Puzilla.mak" CFG="Puzilla - Win32 Debug"
!MESSAGE "Puzilla - Win32 Release" (based on  "Win32 (x86) Console Application")
!MESSAGE "Puzilla - Win32 Debug" (based on  "Win32 (x86) Console Application")
!MESSAGE "Puzilla - NACL Release" (based on  "Win32 (x86) External Target")
!MESSAGE "Puzilla - NACL Debug" (based on  "Win32 (x86) External Target")
!MESSAGE "Puzilla - Emscripten Release" (based on  "Win32 (x86) External Target")
!MESSAGE "Puzilla - Emscripten Debug" (based on  "Win32 (x86) External Target")
!MESSAGE "Puzilla - Android Release" (based on  "Win32 (x86) External Target")
!MESSAGE "Puzilla - Android Debug" (based on  "Win32 (x86) External Target")
# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Puzilla - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release-vc6"
# PROP Intermediate_Dir "Release-vc6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /I "../ZillaLib/Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 "../ZillaLib/Release-vc6/ZillaLib.lib" /nologo /subsystem:windows /pdb:"Release-vc6/Puzilla.pdb" /map:"Release-vc6/Puzilla.map" /machine:I386 /out:"Release-vc6/Puzilla.exe" /opt:ref /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Puzilla - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug-vc6"
# PROP Intermediate_Dir "Debug-vc6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "../ZillaLib/Include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ZILLALOG" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 "../ZillaLib/Debug-vc6/ZillaLib.lib" /nologo /subsystem:console /incremental:yes /pdb:"Debug-vc6/Puzilla.pdb" /debug /machine:I386 /out:"Debug-vc6/Puzilla.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "Puzilla - NACL Release"

# PROP Output_Dir "Release-nacl"
# PROP Intermediate_Dir "Release-nacl"
# PROP Cmd_Line "python ../ZillaLib/NACL/ZillaLibNACL.py build -rel -vc Puzilla"
# PROP Rebuild_Opt "-clean"
# PROP Target_File "Release-nacl/Puzilla_x86_64.nexe.gz.exe"

!ELSEIF  "$(CFG)" == "Puzilla - NACL Debug"

# PROP Output_Dir "Debug-nacl"
# PROP Intermediate_Dir "Debug-nacl"
# PROP Cmd_Line "python ../ZillaLib/NACL/ZillaLibNACL.py build -vc Puzilla"
# PROP Rebuild_Opt "-clean"
# PROP Target_File "Debug-nacl/Puzilla_x86_64.nexe.gz.exe"

!ELSEIF  "$(CFG)" == "Puzilla - Emscripten Release"

# PROP Output_Dir "Release-emscripten"
# PROP Intermediate_Dir "Release-emscripten"
# PROP Cmd_Line "python ../ZillaLib/Emscripten/ZillaLibEmscripten.py build -rel -vc Puzilla"
# PROP Rebuild_Opt "-clean"
# PROP Target_File "Release-emscripten/Puzilla.js.exe"

!ELSEIF  "$(CFG)" == "Puzilla - Emscripten Debug"

# PROP Output_Dir "Debug-emscripten"
# PROP Intermediate_Dir "Debug-emscripten"
# PROP Cmd_Line "python ../ZillaLib/Emscripten/ZillaLibEmscripten.py build -vc Puzilla"
# PROP Rebuild_Opt "-clean"
# PROP Target_File "Debug-emscripten/Puzilla.js.exe"

!ELSEIF  "$(CFG)" == "Puzilla - Android Release"

# PROP Output_Dir "Android/bin"
# PROP Intermediate_Dir "Android/obj"
# PROP Cmd_Line "../ZillaLib/Tools/make.exe --no-print-directory -f ../ZillaLib/Android/ZillaLibAndroid.mk ZLDEBUG=0 ZillaApp=Puzilla"
# PROP Rebuild_Opt "-B"
# PROP Target_File "Android/bin/Puzilla.apk"

!ELSEIF  "$(CFG)" == "Puzilla - Android Debug"

# PROP Output_Dir "Android/bin"
# PROP Intermediate_Dir "Android/obj"
# PROP Cmd_Line "../ZillaLib/Tools/make.exe --no-print-directory -f ../ZillaLib/Android/ZillaLibAndroid.mk ZLDEBUG=1 ZillaApp=Puzilla"
# PROP Rebuild_Opt "-B"
# PROP Target_File "Android/bin/Puzilla.apk"

!ENDIF

# Begin Target
# Name "Puzilla - Win32 Release"
# Name "Puzilla - Win32 Debug"
# Name "Puzilla - NACL Release"
# Name "Puzilla - NACL Debug"
# Name "Puzilla - Emscripten Release"
# Name "Puzilla - Emscripten Debug"
# Name "Puzilla - Android Release"
# Name "Puzilla - Android Debug"
# Begin Source File
SOURCE=./include.h
# End Source File
# Begin Source File
SOURCE=./main.cpp
# End Source File
# Begin Source File
SOURCE=./SceneGame.cpp
# End Source File
# Begin Source File
SOURCE=./SceneTitle.cpp
# End Source File
# Begin Source File
SOURCE=./world.cpp
# End Source File
# Begin Source File
SOURCE=./world.h
# End Source File
# Begin Source File
SOURCE=./../ZillaLib/Opt/chipmunk/chipmunk.cpp
# End Source File
# Begin Source File
SOURCE=./Puzilla.rc
# End Source File
# End Target
# End Project
