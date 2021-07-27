@echo off
set DAT=%1
set target=%2

if not exist ".\buildCodeGen\%target%\" mkdir ".\buildCodeGen\%target%\"

if %errorlevel% NEQ 0 (
	echo "Error cannot create the directory for buildsystem."
	EXIT \B %errorlevel%
)

set DIR=%cd:\=/%

cd .\buildCodeGen\%target%


cmake -DDIR="%DIR%"  %DAT%codeGen\%target%

cmake --build . --target %target%