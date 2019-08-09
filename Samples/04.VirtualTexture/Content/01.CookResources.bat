@echo off & setlocal enabledelayedexpansion

rem Cook to Cooked/Windows directory

set CurrPath=%CD%
set BinaryPath=%CurrPath%\..\..\..\Binary\Windows
set path=%path%;%BinaryPath%
set Converter=%BinaryPath%\02.ResConverter.exe
set VTBaker=%BinaryPath%\04.VTTextureBaker.exe

if not exist "Cooked" (
	mkdir Cooked
	pushd "Cooked"
	if not exist "Windows" (
		mkdir "Windows"
	)
	popd
)

echo Converting tjs files.
for /r . %%i in (*.tjs) do (
  set B=%%i
  set Source=!B:%CD%\=!
  set Target=!Source:~0,-4!
  echo converting - !Source!
  %Converter% !Source! Cooked\Windows\!Target!.tasset -ForceAlphaChannel
)

rem %VTBaker% showcase_01.tjs Cooked/Windows -DumpAllVTs -DumpAllVTWithBorder -DumpAllPages -IgnoreBorders -DebugBorders -VTSize=16384 -PPSize=256
%VTBaker% showcase_01.tjs Cooked/Windows
rem Convert scene file again with vt_info
%Converter% showcase_01.tjs Cooked\Windows\showcase_01.tasset -VTInfo=showcase_01_vt.tjs

echo copy Config
pushd "Cooked\Windows"
if not exist "Config" (
	mkdir "Config"
)
popd
copy Config\*.ini Cooked\Windows\Config\

echo copy others data files
for %%i in (*.bn) do (
echo copying - %%i
copy %%i Cooked\Windows\%%i
)

pause