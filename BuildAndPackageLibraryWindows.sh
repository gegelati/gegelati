#!/bin/bash
# This program builds the TPG library using CMake + MSVC 2019, and packages it in a zip file.
# To work, download zip.exe from http://stahlworks.com/dev/index.php?tool=zipunzip into the bash install.

# Delete previous bin content.
rm -rf bin

# Call CMake (pipe to skip the pause)
true | ./CMakeVS2019.bat

# Build the library
cmake --build ./bin --config Debug --target tpglib
cmake --build ./bin --config Release --target tpglib

# Install the library
cmake --build ./bin --config Release --target INSTALL
cmake --build ./bin --config Debug --target INSTALL

# Package into zip
#cd ..
#zip tpg/tpglib.zip    \
#	tpg/include/*     \
#	tpg/doc/*         \
#	tpg/bin/Debug/*   \
#	tpg/bin/Release/* 
	