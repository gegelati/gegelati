set DIR=%1
set DAT=%2
set target=%3

set build=%DIR%/buildCodeGen/%target%

if not exist %build% mkdir %build%

pushd %DIR%/buildCodeGen/%target%

cmake -DDIR=%DIR% %DAT%/codeGen/%target%

popd

cmake --build %build% --target %target%