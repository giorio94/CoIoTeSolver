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
SET SRCDIR=%CURDIR%\..\src
SET OBJDIR=%CURDIR%\..\obj

FOR %%I IN ("%EXE%") DO (SET EXE=%%~fI)
FOR %%I IN ("%SRCDIR%") DO (SET SRCDIR=%%~fI)
FOR %%I IN ("%OBJDIR%") DO (SET OBJDIR=%%~fI)

SET CXX=g++
SET CXXFLAGS=-Wall -O3 -std=c++11
SET LIBS=-pthread
SET SRCFILE=main coiote_solver_io coiote_solver_logic

IF NOT EXIST "%SRCDIR%\" (
	ECHO The source directory does not exist. ABORT
	EXIT /B 1
)
IF NOT EXIST "%OBJDIR%\" (MKDIR "%OBJDIR%") ELSE (DEL /Q "%OBJDIR%\*")
IF EXIST "%EXE%" (DEL "%EXE%")

FOR %%F IN (%SRCFILE%) DO (
	ECHO %CXX% -c %CXXFLAGS% -o %OBJDIR%\%%F.o %SRCDIR%\%%F.cpp
	"%CXX%" -c %CXXFLAGS% -o "%OBJDIR%\%%F.o" "%SRCDIR%\%%F.cpp"
)
ECHO %CXX% -o %EXE% %OBJDIR%\* %LIBS%
"%CXX%" -o "%EXE%" "%OBJDIR%\*" %LIBS%

ENDLOCAL
EXIT /B 0
