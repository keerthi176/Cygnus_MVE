@echo off
rem Run this script for intallation when needed in python mode
SET CURRENT_DIR=%cd%
python.exe "%CURRENT_DIR%\\setup.py" install
pause