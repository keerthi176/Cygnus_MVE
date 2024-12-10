@echo off

SET SREC_LOC=..\..\..\ThirdParty\srecord
SET KEIL_LOC="C:\Keil_v5"

if exist "..\Application\MCU.uvprojx" (
    @echo Building the ForBootloader target of your project
    
    if exist "..\Application\MCU_ForBootloader\MCU_ForBootloader_CRC.hex"  del "..\Application\MCU_ForBootloader\MCU_ForBootloader_CRC.hex"
      
    %KEIL_LOC%\UV4\UV4.exe -c "..\Application\MCU.uvprojx" -j0 -t "ForBootloader"
    %KEIL_LOC%\UV4\UV4.exe -b "..\Application\MCU.uvprojx" -j0 -t "ForBootloader"
    
    if exist "..\Application\MCU_ForBootloader\MCU_ForBootloader_CRC.hex" (
        %SREC_LOC%\srec_cat.exe .\bootloader\Bootloader.hex -Intel ..\Application\MCU_ForBootloader\MCU_ForBootloader_CRC.hex -Intel -o .\bootloader\BootloaderAndApplication.hex -Intel
        
        if exist ".\bootloader\BootloaderAndApplication.hex" (
            exit 0
        ) else (
            @echo "Merge of the Application and the bootloader has failed"
            exit 1
        )
        
    ) else (
        @echo "Build of MCU_ForBootloader Target of the Application has failed"
        exit 1
    )       
) else (
 @echo "COULDN'T FIND THE Application\MCU.uvprojx PROJECT FILE"
)

goto:eof