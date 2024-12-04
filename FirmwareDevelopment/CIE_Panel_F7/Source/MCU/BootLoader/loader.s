
	AREA bootloader_area, DATA, READONLY
		
bootloader
	EXPORT bootloader
	
	INCBIN ..\DFUMode\build\Objects\DFUMode.bin

	END
		
	
