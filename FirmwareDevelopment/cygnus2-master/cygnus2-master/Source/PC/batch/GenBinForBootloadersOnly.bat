@echo off

SET SREC_LOC=..\..\ThirdParty\srecord
SET BOOTLOADER_LOC=..\..\..\RBUFrontBootloader\Objects
SET PPU_UPDATE_LOC=..\..\..\PPU\PPU\Devlopment\RBUPPUModeBootloader\build\MinSizeRel
for /f "delims=" %%F in ('where ST-LINK_CLI.exe') do set ST_LINK_UTIL_CLI=%%F 

rem concatinate the bootloader, PPU updater and RBU images for production programming
if exist "%BOOTLOADER_LOC%\Bootloader_CHKSM.hex" (
	if exist "%PPU_UPDATE_LOC%\RBUPPUModeBootloader.hex" (
		rem Crop the bootloader file from 0x08000000 to 0x08004000
		%SREC_LOC%\srec_cat.exe "%BOOTLOADER_LOC%\Bootloader_CHKSM.hex" -Intel -crop 0x08000000 0x08004000 -o ".\%1\Bootloader_crop.hex" -Intel
		if exist ".\%1\Bootloader_crop.hex" (
			rem Crop the ppu_update file from 0x08004000 to 0x08010000
			%SREC_LOC%\srec_cat.exe "%PPU_UPDATE_LOC%\RBUPPUModeBootloader.hex" -Intel -crop 0x08004000 0x08020000 -o ".\%1\PPU_UPDATE_crop.hex" -Intel
			if exist ".\%1\PPU_UPDATE_crop.hex" (
				rem merge the bootloader, ppu_update and RBU code into a single file
				%SREC_LOC%\srec_cat.exe  ".\%1\Bootloader_crop.hex" -Intel ".\%1\PPU_UPDATE_crop.hex" -Intel   -o ".\%1\Production_Load_Bootloaders.hex" -Intel
				if exist ".\%1\Production_Load_Bootloaders.hex" (
					rem Crop the file from 0x08000000 to 0x08020000
					%SREC_LOC%\srec_cat.exe .\%1\Production_Load_Bootloaders.hex -intel -crop 0x08000000 0x08020000 -checksum-positive-l-e 0x0801FFFC 4 1 -o ".\%1\Production_Bootloaders_crop.hex" -Intel
					if exist ".\%1\Production_Bootloaders_crop.hex" (
						rem @echo Offset the Hex file by the start Address 0x08000000
						%SREC_LOC%\srec_cat.exe ".\%1\Production_Bootloaders_crop.hex" -Intel -offset -0x08000000 -o ".\%1\Production_Bootloaders.hex" -Intel
						if exist ".\%1\Production_Bootloaders.hex" (
							rem Convert the hex file to a binary
							%SREC_LOC%\srec_cat.exe ".\%1\Production_Bootloaders.hex" -Intel -o .\%1\Production_Bootloaders.bin -Binary
						) else (
							echo "COULDN'T FIND FILE Production_Bootloaders.hex"
						) 
					) else (
						echo "COULDN'T FIND FILE Production_Bootloaders_crop.hex"
					)
				) else (
					echo "COULDN'T FIND FILE Production_Load_Offset.hex"
				)
			) else (
				echo "COULDN'T FIND FILE PPU_UPDATE_crop.hex"
			)
		) else (
			echo "COULDN'T FIND FILE Bootloader_crop.hex"
		)
	) else (
		echo "COULDN'T FIND FILE %PPU_UPDATE_LOC%\RBUPpuBootloader.hex"
	)
) else (
	echo "COULDN'T FIND FILE Bootloader_CHKSM.hex"
)
goto:del_temp_files


:del_temp_files  
  if exist ".\%1\Bootloader_crop.hex" del ".\%1\Bootloader_crop.hex"
  rem if exist ".\%1\Production_Bootloaders.hex" del ".\%1\Production_Bootloaders.hex"
  if exist ".\%1\Production_Load_Offset.hex" del ".\%1\Production_Load_Offset.hex"
  if exist ".\%1\Production_Bootloaders_crop.hex" del ".\%1\Production_Bootloaders_crop.hex"
  if exist ".\%1\Production_Load_Bootloaders.hex" del ".\%1\Production_Load_Bootloaders.hex"
  if exist ".\%1\PPU_UPDATE_crop.hex" del ".\%1\PPU_UPDATE_crop.hex"
goto:eof