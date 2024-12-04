@echo off
SETLOCAL EnableDelayedExpansion

set out_file=%EmWi_OutputDirectory%\ewfiles.gpdsc

if not exist %EmWi_OutputDirectory%\..\PlatformPackage\%EmWi_ColorFormat% (
echo "ERROR: The requested color format %EmWi_ColorFormat% is not supported for this platform!"
exit /B 1
)

set rte_path=..\..\PlatformPackage\RTE
set gfx_path=..\..\PlatformPackage\%EmWi_ColorFormat%

date /t > date.tmp
time /t > time.tmp
for /f %%i IN (date.tmp) DO SET cur_date=%%i
for /f %%i IN (time.tmp) DO SET cur_time=%%i
del date.tmp time.tmp


if exist %EmWi_OutputDirectory%/../../PlatformPackage/RTE/ewcolor.c (
  set EmWi_RTESrcFiles=ewcolor.c ewdebug.c ewextrte.c ewobject.c ewpoint.c ewrect.c ewref.c ewresource.c ewscalars.c ewslot.c ewstring.c ewtimer.c
  
  if %EmWi_ColorFormat% == Index8 (
    set EmWi_GFX_SrcFiles=ewextbmp_Index8.c ewextfnt.c ewextgfx.c ewextpxl_Index8.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c
  )
  if %EmWi_ColorFormat% == LumA44 (
    set EmWi_GFX_SrcFiles=ewextbmp_LumA44.c ewextfnt.c ewextgfx.c ewextpxl_LumA44.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c
  )
  if %EmWi_ColorFormat% == RGB565 (
    set EmWi_GFX_SrcFiles=ewextbmp_RGB565_RGBA8888.c ewextfnt.c ewextgfx.c ewextpxl_RGB565_RGBA8888.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c
  )
  if %EmWi_ColorFormat% == RGB888 (
    set EmWi_GFX_SrcFiles=ewextbmp_RGB888_RGBA8888.c ewextfnt.c ewextgfx.c ewextpxl_RGB888_RGBA8888.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c
  )
  if %EmWi_ColorFormat% == RGBA4444 (
    set EmWi_GFX_SrcFiles=ewextbmp_RGBA4444.c ewextfnt.c ewextgfx.c ewextpxl_RGBA4444.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c
  )
  if %EmWi_ColorFormat% == RGBA8888 (
    set EmWi_GFX_SrcFiles=ewextbmp_RGBA8888.c ewextfnt.c ewextgfx.c ewextpxl_RGBA8888.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c
  )
) ELSE (
  set EmWi_RTESrcFiles=ewextrte.c
  set EmWi_RTELibFiles=libewrte-m4-keil.lib
  if %EmWi_SurfaceRotation% == 0 (
    set EmWi_GFX_SrcFiles=ewextgfx.c
    set EmWi_GFX_LibFiles=libewgfx-m4-keil.lib
  ) else (
    set EmWi_GFX_SrcFiles=ewextgfx.c
    set EmWi_GFX_LibFiles=libewgfx-m4-r%EmWi_SurfaceRotation%-keil.lib
  )
)



@echo ^<?xml version="1.0" encoding="UTF-8"?^> > %out_file%
@echo ^<package xmlns:xs="http://www.w3.org/2001/XMLSchema-instance" schemaVersion="1.0" xs:noNamespaceSchemaLocation="PACK.xsd"^> >> %out_file%
@echo   ^<vendor^>TARA Systems^</vendor^> >> %out_file%
@echo   ^<name^>EmWiGeneratedCode^</name^> >> %out_file%
@echo   ^<description^>Generated Code of Embedded Wizard^</description^> >> %out_file%
@echo   ^<url^>^</url^> >> %out_file%
@echo   ^<releases^> >> %out_file%
@echo     ^<release version="1.0.0"^> >> %out_file%
@echo      - Generated: %cur_date% %cur_time% >> %out_file%
@echo     ^</release^> >> %out_file%
@echo   ^</releases^> >> %out_file%

@echo     ^<generators^> >> %out_file%
@echo       ^<generator id="Embedded_Wizard"^> >> %out_file%
@echo         ^<description^>Tara: Embedded Wizard Studio^</description^> >> %out_file%
@echo         ^<command^>%EmWi_InstallDirectory%\EmbeddedWizard.exe^</command^> >> %out_file%
@echo         ^<workingDir^>%EmWi_InstallDirectory%^</workingDir^> >> %out_file%
@echo         ^<arguments^> >> %out_file%
@echo           ^<argument^>%EmWi_ProjectFile%^</argument^> >> %out_file%
@echo         ^</arguments^> >> %out_file%
@echo         ^<gpdsc name="$P../../GeneratedCode/ewfiles.gpdsc"/^> >> %out_file%
@echo       ^</generator^> >> %out_file%
@echo     ^</generators^>  >> %out_file% 

@echo   ^<components^> >> %out_file%

@echo     ^<component generator="Embedded_Wizard" Cclass="EmWi/GeneratedCode" Cgroup="Open Project "^> >> %out_file%
@echo       ^<description^>Opens the connected Embedded Wizard Project^</description^> >> %out_file%
@echo     ^</component^> >> %out_file%

@echo     ^<component Cclass="EmWi/GeneratedCode" Cgroup="Generated Code"^> >> %out_file%
@echo       ^<description^>Embedded Wizard Generated Source Code^</description^> >> %out_file%
@echo       ^<RTE_Components_h^> >> %out_file%
@echo          #define  EW_SURFACE_ROTATION %EmWi_SurfaceRotation% >> %out_file%
@echo          #define  EW_FRAME_BUFFER_COLOR_FORMAT EW_FRAME_BUFFER_COLOR_FORMAT_%EmWi_ColorFormat% >> %out_file%
@echo       ^</RTE_Components_h^> >> %out_file%
@echo       ^<files^> >> %out_file%

for %%f in (%EmWi_OutputSrcFiles%) do ( @echo         ^<file category="source" name="%%f"/^> >> %out_file% )

@echo       ^</files^> >> %out_file%
@echo     ^</component^> >> %out_file%

@echo     ^<component Cclass="EmWi/PlatformPackage" Cgroup="Platform Package"^> >> %out_file%
@echo       ^<description^>Embedded Wizard Platform Package Libs and Source Code^</description^> >> %out_file%
@echo       ^<RTE_Components_h^> >> %out_file%
@echo       ^</RTE_Components_h^> >> %out_file%
@echo       ^<files^> >> %out_file%

@echo       ^<^^!--  include folder --^> >> %out_file%
@echo         ^<file category="include" name="..\..\PlatformPackage/RTE/"/^> >> %out_file%
@echo         ^<file category="include" name="..\..\PlatformPackage/%EmWi_ColorFormat%/"/^> >> %out_file%

for %%f in (%EmWi_RTESrcFiles%) do ( @echo         ^<file category="source" name="%rte_path%\%%f"/^> >> %out_file% )
for %%f in (%EmWi_GFX_SrcFiles%) do ( @echo         ^<file category="source" name="%gfx_path%\%%f"/^> >> %out_file% )
for %%f in (%EmWi_RTELibFiles%) do ( @echo         ^<file category="library" name="%rte_path%\%%f"/^> >> %out_file% )
for %%f in (%EmWi_GFX_LibFiles%) do ( @echo         ^<file category="library" name="%gfx_path%\%%f"/^> >> %out_file% )

@echo       ^</files^> >> %out_file%
@echo     ^</component^> >> %out_file%


@echo   ^</components^> >> %out_file%
  
@echo ^</package^> >> %out_file%