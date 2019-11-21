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
  %Converter% !Source! Cooked\Windows\!Target!.tasset -ForceAlphaChannel -cluster_size=128 -cluster_verbose
)

rem %VTBaker% showcase_01.tjs Cooked/Windows -DumpAllVTs -DumpAllVTWithBorder -DumpAllPages -IgnoreBorders -DebugBorders -VTSize=16384 -PPSize=256
rem %VTBaker% showcase_04.tjs Cooked/Windows
rem Convert scene file again with vt_info
rem %Converter% showcase_04.tjs Cooked\Windows\showcase_04.tasset -VTInfo=showcase_04_vt.tjs

echo copy Config
pushd "Cooked\Windows"
if not exist "Config" (
	mkdir "Config"
)
popd
copy Config\*.ini Cooked\Windows\Config\

pause