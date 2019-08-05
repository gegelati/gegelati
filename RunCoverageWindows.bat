OpenCppCoverage.exe --source %cd% --excluded_source  %cd%\bin --export_type html:bin\coverage --excluded_line_regex "\s*else.*" --excluded_line_regex "\s*\{.*" .\bin\Debug\runTests.exe

.\bin\coverage\index.html

pause