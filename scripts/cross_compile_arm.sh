#!/bin/bash

PRINTED_MAKEFILE=Makefile
COMPILER=arm-linux-gnueabihf-g++
INSTALL_DIR=./rootfs

## Create Makefile
echo "Create Makefile"

# Go back to bin folder
DIR=$(cd `dirname $0` && echo `git rev-parse --show-toplevel`)
cd $DIR/bin
# echo "$(pwd)"

# Get include directories
INCLUDE_FOLDER=$(find "../gegelatilib/include" -type d -printf "%p ")

# Get CPP directories
CPP_FOLDER=$(find "../gegelatilib/src" -type d -printf "%p/:")

# Get all header files
H_FILES=$(find "../gegelatilib/" -regex ".*\.h" -type f -printf "%f ";)

# Get all cpp files
CPP_FILES=$(find "../gegelatilib/" -regex ".*\.cpp" -type f -exec basename {} \;)

# Start printing
rm $PRINTED_MAKEFILE
echo "INCLUDE_DIRECTORIES=$INCLUDE_FOLDER" >> $PRINTED_MAKEFILE
echo "vpath %.h \$(INCLUDE_DIRECTORIES)" >> $PRINTED_MAKEFILE
echo "CC=$COMPILER --std=c++17" >> $PRINTED_MAKEFILE
echo "CFLAGS=\$(addprefix -I ,\$(INCLUDE_DIRECTORIES)) -fPIC" >> $PRINTED_MAKEFILE
echo "LDFLAGS=-pthread -shared" >> $PRINTED_MAKEFILE
echo "VPATH=$CPP_FOLDER" >> $PRINTED_MAKEFILE

echo "" >> $PRINTED_MAKEFILE
echo "OBJS=$(echo $CPP_FILES | sed 's/.cpp/.o/g')" >> $PRINTED_MAKEFILE

echo "
all: libgegelati.so

libgegelati.so: \$(OBJS)
	\$(CC) \$^ -o \$@ \$(LDFLAGS)
	@mkdir ./obj
	@mv *.o ./obj" >> $PRINTED_MAKEFILE

# Print dependencies (exhaustively for now)
for CPP_FILE in $CPP_FILES; do
	echo "$(echo $CPP_FILE | sed 's/.cpp/.o/g'): $H_FILES" >> $PRINTED_MAKEFILE
done

echo "

%.o: %.cpp
	\$(CC) \$(CFLAGS) -c \$<

install:
	install -d $INSTALL_DIR/usr/local/lib/gegelati
	mv libgegelati.so $INSTALL_DIR/usr/local/lib/gegelati
	install -d $INSTALL_DIR/usr/local/include/gegelati
	cp -rf ./gegelatilib/include/* $INSTALL_DIR/usr/local/include/gegelati

clean:
	@rm -f bin/* gegelatilib.so
	@rm -f *.o *.gcno *.gcda *.save
	@rm -r ./obj
" >> $PRINTED_MAKEFILE