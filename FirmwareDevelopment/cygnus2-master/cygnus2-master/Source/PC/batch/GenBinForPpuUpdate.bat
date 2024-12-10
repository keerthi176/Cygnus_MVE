@echo off

SET SREC_LOC=..\..\ThirdParty\srecord
for /f "delims=" %%F in ('where ST-LINK_CLI.exe') do set ST_LINK_UTIL_CLI=%%F 

if exist ".\Debug\MCU_PpuBootloader.hex" (

	rem Crop off the first 16K (reserved for bootloader App)
	%SREC_LOC%\srec_cat.exe .\Debug\MCU_PpuBootloader.hex -intel -crop 0x08004000 0x08020000 -offset -0x00004000 -o .\Debug\ppu_bootloader_OFFSET.hex -Intel
	rem Calculate the checksum and place it in the correct location 
	%SREC_LOC%\srec_cat.exe .\Debug\ppu_bootloader_OFFSET.hex -intel -crop 0x08000000 0x0801BFFC -checksum-positive-l-e 0x0801BFFC 4 1 -o .\Debug\ppu_bootloader_CHKSM.hex -Intel

	if exist "%ST_LINK_UTIL_CLI%" (
		rem "%ST_LINK_UTIL_CLI%" -Cksum ".\%1\MCU_%1.hex"	
		"%ST_LINK_UTIL_CLI%" -Cksum ".\Debug\ppu_bootloader_CHKSM.hex"
		)
	
	if exist ".\Debug\ppu_bootloader_CHKSM.hex" (
		rem @echo Offset the Hex file by the start Address 0x08000000
		%SREC_LOC%\srec_cat.exe ".\Debug\ppu_bootloader_CHKSM.hex" -Intel -offset -0x08000000 -o ".\Debug\ppu_bootloader_CHKSM_Offset.hex" -Intel
	
		if exist ".\Debug\ppu_bootloader_CHKSM_Offset.hex" (
			rem @echo Converting the HEX file to Binary
			%SREC_LOC%\srec_cat.exe .\Debug\ppu_bootloader_CHKSM_Offset.hex -Intel -o .\Debug\MCU_PPU_Bootloader.bin -Binary
		) else (
		    echo "COULDN'T FIND FILE .\Debug\ppu_bootloader_CHKSM_Offset.hex"
			goto:del_temp_files
		)
	) else (
		echo "COULDN'T FIND FILE .\Debug\ppu_bootloader_CHKSM.hex"
		goto:del_temp_files
	)
) else (
	echo "COULDN'T FIND FILE .\Debug\MCU_PpuBootloader.hex"
	goto:del_temp_files
)



:del_temp_files  
  if exist ".\Debug\ppu_bootloader_OFFSET.hex"  del ".\Debug\ppu_bootloader_OFFSET.hex" 
  if exist ".\Debug\ppu_bootloader_CHKSM.hex"  del ".\Debug\ppu_bootloader_CHKSM.hex" 
  if exist ".\Debug\ppu_bootloader_CHKSM_Offset.hex"  del ".\Debug\ppu_bootloader_CHKSM_Offset.hex" 
goto:eof