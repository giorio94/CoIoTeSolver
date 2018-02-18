:: This file is part of CoIoTeSolver.

:: CoIoTeSolver is free software: you can redistribute it and/or modify
:: it under the terms of the GNU General Public License as published by
:: the Free Software Foundation, either version 3 of the License, or
:: (at your option) any later version.

:: CoIoTeSolver is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
:: GNU General Public License for more details.

:: You should have received a copy of the GNU General Public License
:: along with CoIoTeSolver. If not, see <http://www.gnu.org/licenses/>.


@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

SET CURDIR=%~dp0
SET EXE=%CURDIR%\..\CoIoTeSolver.exe
SET DEFDIR=%CURDIR%\..\input
SET OUTDIR=%CURDIR%\..\output
SET SUMMARY=summary.csv

SET INDIR=%1
IF "%INDIR%"=="" (SET INDIR=%DEFDIR%)
FOR %%I IN ("%INDIR%") DO (SET INDIR=%%~fI)
FOR %%I IN ("%EXE%") DO (SET EXE=%%~fI)
FOR %%I IN ("%OUTDIR%") DO (SET OBJDIR=%%~fI)

CLS
ECHO Selected input directory: '%INDIR%'

IF NOT EXIST "%INDIR%\" (
	ECHO The selected directory does not exist. ABORT
	EXIT /B 1
)

ECHO If correct press Y to continue, otherwise press N and run the
ECHO script again specifying the correct directory as parameter.
SET /p ans=Is this directory correct? (Y/N)

IF /I "%ans%"=="y" (
	IF NOT EXIST "%OUTDIR%\" (
		MKDIR "%OUTDIR%"
	) ELSE (
		DEL /Q "%OUTDIR%\*"
	)

	FOR %%F in ("%INDIR%\*") DO (
		SET FNAME=%%~nxF
		<NUL SET /p=!FNAME!:
		"%EXE%" "%%F" "%OUTDIR%\%SUMMARY%" "%OUTDIR%\!FNAME!" --test
	)
)

ENDLOCAL
EXIT /B 0


