@echo off

SET SREC_LOC=..\..\ThirdParty\srecord
for /f "delims=" %%F in ('where ST-LINK_CLI.exe') do set ST_LINK_UTIL_CLI=%%F 

if exist ".\%1\MCU_%1.hex" (
	rem @echo Generating Hex file with CRC value
	rem %SREC_LOC%\srec_cat.exe .\%1\MCU_%1.hex -intel -crop 0x08000000 0x0803DFFC -fill 0x00 0x08000000 0x0803DFFC -STM32 0x0803DFFC -o .\%1\MCU_%1_CRC.hex -Intel
	rem @echo Application Checksum Value in Intel Format:
	rem %SREC_LOC%\srec_cat.exe .\%1\MCU_%1_CRC.hex -intel -crop 0x0803DFFC 0x0803E000 -Output - -hex-dump 

	%SREC_LOC%\srec_cat.exe .\%1\MCU_%1.hex -intel -crop 0x08000000 0x08060000 -fill 0xFF 0x08000000 0x08060000 -checksum-positive-l-e 0x08060000 4 1 -o .\%1\MCU_%1_CHKSM.hex -Intel

	if exist "%ST_LINK_UTIL_CLI%" (
		rem "%ST_LINK_UTIL_CLI%" -Cksum ".\%1\MCU_%1.hex"	
		"%ST_LINK_UTIL_CLI%" -Cksum ".\%1\MCU_%1_CHKSM.hex"	
		)
	
	if exist ".\%1\MCU_%1_CHKSM.hex" (
		rem @echo Offset the Hex file by the start Address 0x0800C000
		%SREC_LOC%\srec_cat.exe ".\%1\MCU_%1_CHKSM.hex" -Intel -offset -0x08000000 -o ".\%1\MCU_%1_CHKSM_Offset.hex" -Intel
	
		if exist ".\%1\MCU_%1_CHKSM_Offset.hex" (
			rem @echo Converting the HEX file to Binary
			%SREC_LOC%\srec_cat.exe .\%1\MCU_%1_CHKSM_Offset.hex -Intel -o .\%1\MCU_%1.bin -Binary
			goto:del_temp_files
		) else (
		    echo "COULDN'T FIND THE MCU_%1_CHKSM_Offset.hex FILE"
			goto:del_temp_files
		)
	) else (
		echo "COULDN'T FIND THE MCU_%1_CHKSM.hex FILE"
		goto:del_temp_files
	)
) else (
 @echo "COULDN'T FIND THE .\%1\MCU_%1.hex FILE"
)

:del_temp_files  
  if exist ".\%1\MCU_%1.hex"  del ".\%1\MCU_%1.hex" 
  if exist ".\%1\MCU_%1_CHKSM_Offset.hex"  del ".\%1\MCU_%1_CHKSM_Offset.hex" 
goto:eof