#!/bin/bash
set -e

# Build and run tests
cmake -S . -B bin -DCMAKE_BUILD_TYPE=Debug
cmake --build bin --target runTests -j 18
./bin/bin/runTests

# Create coverage directory
mkdir -p bin/coverage

# Generate coverage report ONLY for gegelatilib
gcovr \
    -r . \
    --filter 'gegelatilib/' \
    -s \
    --html \
    --html-details \
    -o bin/coverage/coverage.html