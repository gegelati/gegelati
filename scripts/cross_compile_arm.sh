#!/bin/bash

PRINTED_MAKEFILE=Makefile

## Create Makefile
echo "Create Makefile"

# Go back to bin folder
DIR=$(cd `dirname $0` && echo `git rev-parse --show-toplevel`)
cd $DIR/bin
# echo "$(pwd)"

# Get include directories
INCLUDE_FOLDER=$(find "../gegelatilib/include" -type d -printf "%p ")

# Get all header files
H_FILES=$(find "../gegelatilib/" -regex ".*\.h" -type f -exec basename {} \;)

# Get all cpp files
CPP_FILES=$(find "../gegelatilib/" -regex ".*\.cpp" -type f -exec basename {} \;)

# Start printing
rm $PRINTED_MAKEFILE
echo "INCLUDE_DIRECTORIES=$INCLUDE_FOLDER" >> $PRINTED_MAKEFILE
echo "vpath %.h \$(INCLUDE_DIRECTORIES)" >> $PRINTED_MAKEFILE




