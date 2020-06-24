OpenCppCoverage.exe --source %cd% --excluded_source  %cd%\bin --excluded_source  %cd%\lib --export_type html:bin\coverage --excluded_line_regex "\s*else.*" --excluded_line_regex "\s*\{.*" --excluded_line_regex "\s*\}.*" --excluded_sources "*\deterministicRandom.h" .\bin\bin\Debug\runTests.exe

.\bin\coverage\index.html

pause