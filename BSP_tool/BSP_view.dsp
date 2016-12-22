# Microsoft Developer Studio Project File - Name="BSP_view" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=BSP_VIEW - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "BSP_view.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BSP_view.mak" CFG="BSP_VIEW - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BSP_view - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "BSP_view - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BSP_view - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib glu32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /machine:I386

!ELSEIF  "$(CFG)" == "BSP_view - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "BSP_view - Win32 Release"
# Name "BSP_view - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BSP_view.cpp
# End Source File
# Begin Source File

SOURCE=.\bspfile.cpp
# End Source File
# Begin Source File

SOURCE=.\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\cmdlib.cpp
# End Source File
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=.\entity.cpp
# End Source File
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\glwin.cpp
# End Source File
# Begin Source File

SOURCE=.\graphics.cpp
# End Source File
# Begin Source File

SOURCE=.\mathlib.cpp
# End Source File
# Begin Source File

SOURCE=.\paklib.cpp
# End Source File
# Begin Source File

SOURCE=.\render.cpp
# End Source File
# Begin Source File

SOURCE=.\scriplib.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_render.cpp
# End Source File
# Begin Source File

SOURCE=.\studio_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\sys_time.cpp
# End Source File
# Begin Source File

SOURCE=.\texture.cpp
# End Source File
# Begin Source File

SOURCE=.\trace.cpp
# End Source File
# Begin Source File

SOURCE=.\vis.cpp
# End Source File
# Begin Source File

SOURCE=.\wadlib.cpp
# End Source File
# Begin Source File

SOURCE=.\world.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bspfile.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\cmdlib.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\entity.h
# End Source File
# Begin Source File

SOURCE=.\fastmath.h
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\glwin.h
# End Source File
# Begin Source File

SOURCE=.\graphics.h
# End Source File
# Begin Source File

SOURCE=.\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\paklib.h
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\scriplib.h
# End Source File
# Begin Source File

SOURCE=.\studio.h
# End Source File
# Begin Source File

SOURCE=.\studio_model.h
# End Source File
# Begin Source File

SOURCE=.\sys_time.h
# End Source File
# Begin Source File

SOURCE=.\texture.h
# End Source File
# Begin Source File

SOURCE=.\trace.h
# End Source File
# Begin Source File

SOURCE=.\vis.h
# End Source File
# Begin Source File

SOURCE=.\wadlib.h
# End Source File
# Begin Source File

SOURCE=.\world.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
