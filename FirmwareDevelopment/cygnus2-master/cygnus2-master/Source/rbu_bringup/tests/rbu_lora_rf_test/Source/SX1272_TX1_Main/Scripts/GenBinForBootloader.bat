@echo off

SET SREC_LOC=..\..\..\..\..\..\ThirdParty\srecord
for /f "delims=" %%F in ('where ST-LINK_CLI.exe') do set ST_LINK_UTIL_CLI=%%F 

if exist ".\BUILD\SX1272_TX1_Main.hex" (

	rem Crop off the first 16K (reserved for bootloader App)
	rem %SREC_LOC%\srec_cat.exe .\%1\MCU_%1.hex -intel -crop 0x08004000 0x08054000 -fill 0x00 0x08004000 0x08054000 -offset -0x00004000 -o .\%1\MCU_%1_OFFSET.hex -Intel
	%SREC_LOC%\srec_cat.exe .\BUILD\SX1272_TX1_Main.hex -intel -crop 0x08004000 0x08054000 -offset -0x00004000 -o .\BUILD\SX1272_TX1_Main_Offset.hex -Intel
	
	if exist ".\BUILD\SX1272_TX1_Main_Offset.hex" (
		rem @echo Offset the Hex file by the start Address 0x08000000
		%SREC_LOC%\srec_cat.exe ".\BUILD\SX1272_TX1_Main_Offset.hex" -Intel -offset -0x08000000 -o ".\BUILD\SX1272_TX1_Main_Offset1.hex" -Intel
	
		if exist ".\BUILD\SX1272_TX1_Main_Offset1.hex" (
			rem @echo Converting the HEX file to Binary
			%SREC_LOC%\srec_cat.exe .\BUILD\SX1272_TX1_Main_Offset1.hex -Intel -o .\BUILD\SX1272_TX1_Main.bin -Binary
			goto:del_temp_files
		) else (
		    echo "COULDN'T FIND FILE SX1272_TX1_Main_Offset1.hex"
			goto:del_temp_files
		)
	) else (
		echo "COULDN'T FIND FILE SX1272_TX1_Main_Offset.hex"
		goto:del_temp_files
	)
) else (
 @echo "COULDN'T FIND FILE SX1272_TX1_Main.hex"
)

:del_temp_files  
  if exist ".\BUILD\SX1272_TX1_Main_Offset.hex"  del ".\BUILD\SX1272_TX1_Main_Offset.hex" 
  if exist ".\BUILD\SX1272_TX1_Main_Offset1.hex"  del ".\BUILD\SX1272_TX1_Main_Offset1.hex" 
goto:eof