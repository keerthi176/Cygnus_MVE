 @echo off
for /f %%a in ('dir /B /S *.hex') do dfuse.exe %%a %%a.dfu
for /f %%a in ('dir /B /S *.dfu') do DfuSeCommand.exe -c -d --fn %%a
 echo Le répertoire courant est %CD%