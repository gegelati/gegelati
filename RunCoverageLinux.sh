#/bin/bash!

cd bin
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target runTests
./bin/runTests
mkdir coverage
gcovr -r .. -e '.*deterministicRandom.h' -e '.*test/.*' -e '.*lib/.*' --html --html-details -s -o ./coverage/coverage.html

