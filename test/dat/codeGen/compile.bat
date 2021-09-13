@echo off
set DEBUG=%1
set DAT=%2
set target=%3

if not exist ".\buildCodeGen\%target%\" mkdir ".\buildCodeGen\%target%\"

if %errorlevel% NEQ 0 (
	echo "Error cannot create the directory for buildsystem."
	EXIT \B %errorlevel%
)

set DIR=%cd:\=/%

cd .\buildCodeGen\%target%


cmake -DDIR="%DIR%" -DDEBUG=%DEBUG% %DAT%codeGen\%target%

cmake --build . --target %target%