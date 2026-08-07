#!/bin/bash
set -e

rm -rf bin

cmake -S . -B bin -DCMAKE_BUILD_TYPE=Debug
cmake --build bin --target runTests -j 18

./bin/bin/runTests

mkdir -p bin/coverage

gcovr \
    -r . \
    -s \
    --html \
    --html-details \
    -o bin/coverage/coverage.html