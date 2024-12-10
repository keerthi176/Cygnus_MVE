SET CURRENT_DIR=%cd%
pyinstaller.exe --onefile --windowed --clean -F --add-data "%CURRENT_DIR%\\logo\\BP_Logo.gif";. --add-data "%CURRENT_DIR%\\logo\\Cygnus.jpg";. --icon="%CURRENT_DIR%\\logo\\BP_Logo.ico" --paths="%CURRENT_DIR%\\xmodem-ymodem" "%CURRENT_DIR%\\cygnus_gui.pyw"

pause