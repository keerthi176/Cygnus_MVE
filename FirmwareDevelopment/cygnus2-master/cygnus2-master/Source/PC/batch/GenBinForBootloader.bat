@echo off

SET SREC_LOC=..\..\ThirdParty\srecord
SET PPU_UPDATE_LOC=..\..\..\RBUPpuBootloader\Objects
for /f "delims=" %%F in ('where ST-LINK_CLI.exe') do set ST_LINK_UTIL_CLI=%%F 

if exist ".\%1\MCU_%1.hex" (
	rem Crop out the RBU memory range and offset it by -0x00020000 (to 0x08000000)
	%SREC_LOC%\srec_cat.exe .\%1\MCU_%1.hex -intel -crop 0x08020000 0x0807E000 -fill 0xFF 0x08020000 0x0807E000 -offset -0x00020000 -o .\%1\MCU_%1_OFFSET.hex -Intel
	rem Calculate the checksum and place it in the correct location 
	%SREC_LOC%\srec_cat.exe .\%1\MCU_%1_OFFSET.hex -intel -crop 0x08000000 0x0805DFFC -crc32-l-e 0x0805DFFC -o .\%1\MCU_%1_CHKSM.hex -Intel

	if exist "%ST_LINK_UTIL_CLI%" (
		"%ST_LINK_UTIL_CLI%" -Cksum ".\%1\MCU_%1_CHKSM.hex"	
		)
	
	if exist ".\%1\MCU_%1_CHKSM.hex" (
		rem @echo Offset the Hex file by the start Address 0x08000000
		%SREC_LOC%\srec_cat.exe ".\%1\MCU_%1_CHKSM.hex" -Intel -offset -0x08000000 -o ".\%1\MCU_%1_CHKSM_Offset.hex" -Intel
	
		if exist ".\%1\MCU_%1_CHKSM_Offset.hex" (
			rem @echo Converting the HEX file to Binary
			%SREC_LOC%\srec_cat.exe .\%1\MCU_%1_CHKSM_Offset.hex -Intel -o ".\%1\Radio Board %2_%1.ota" -Binary
		) else (
		    echo "COULDN'T FIND THE MCU_%1_CHKSM_Offset.hex FILE"
			goto:del_temp_files
		)
	) else (
		echo "COULDN'T FIND THE MCU_%1_CHKSM.hex FILE"
		goto:del_temp_files
	)
	
	rem concatinate the bootloaders and RBU image for production programming
	if exist ".\%1\Production_Bootloaders.bin" (
		if exist ".\%1\Radio Board %2_%1.ota" (
			%SREC_LOC%\srec_cat.exe ".\%1\Production_Bootloaders.bin" -Binary -fill 0xff 0x00000000 0x00020000 ".\%1\Radio Board %2_%1.ota" -Binary -fill 0x00 0x00020000 0x0007E000 -offset 0x00020000 -crop 0x00000000 0x0007E000 -o ".\%1\Radio Board %2_%1.bin" -Binary
		) else (
			echo "COULDN'T FIND FILE .\%1\Radio Board %2_%1.ota"
		)
	) else (
		echo "COULDN'T FIND FILE .\%1\Production_Bootloaders.bin"
	)
	rem Generate the .cbl by cropping the first bootloader of the production image
	if exist ".\%1\Radio Board %2_%1.bin" (
		%SREC_LOC%\srec_cat.exe ".\%1\Radio Board %2_%1.bin" -Binary -crop 0x00004000 0x0007E000 -offset -0x0004000 -o ".\%1\Radio Board %2_%1.cbl" -Binary
	) else (
		echo "COULDN'T FIND FILE .\%1\Radio Board %2_%1.bin"
	)
) else (
 @echo "COULDN'T FIND THE .\%1\MCU_%1.hex FILE"
)
goto:del_temp_files

:del_temp_files  
if exist ".\%1\MCU_%1_CHKSM.hex"  del ".\%1\MCU_%1_CHKSM.hex" 
if exist ".\%1\MCU_%1_CHKSM_Offset.hex"  del ".\%1\MCU_%1_CHKSM_Offset.hex" 
if exist ".\%1\MCU_%1_OFFSET.hex"  del ".\%1\MCU_%1_OFFSET.hex" 
goto:eof