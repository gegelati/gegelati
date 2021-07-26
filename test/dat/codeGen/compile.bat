DIR=%1
DAT=%2
target=%3

set build=%DIR%/buildCodeGen/%target%

mkdir -p %build%

pushd %DIR%/buildCodeGen/%target%

cmake -DDIR=%DIR% %DAT%/codeGen/%target%

popd

cmake --build %build% --target %target%