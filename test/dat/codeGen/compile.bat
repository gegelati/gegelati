set DIR=%1
set DAT=%2
set target=%3

set build=%DIR%/build/CodeGen/%target%

if not exist %build% mkdir %build%

pushd %build%

cmake -DDIR=%DIR% %DAT%codeGen/%target%

popd

cmake --build %build% --target %target%