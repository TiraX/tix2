@echo off

set CurrPath=%CD%
set BinaryPath=%CurrPath%\..\..\..\Binary
set path=%path%;%BinaryPath%
set Converter=%BinaryPath%\02.ResConverter.exe

for %%i in (*.tjs) do (
echo converting - %%i
%Converter% %%i
)

pause