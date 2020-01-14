#!/bin/bash
set -x

# Get version number from CMakeLists.txt
VERSION_NUMBER="$(cat CMakeLists.txt | grep -m 1 "VERSION \([0-9]\+.[0-9]\+.[0-9]\+\)" | sed -E -e 's/ |\)|VERSION//g')"

# Prepare folder
mkdir neutral_builds
cd neutral_builds


# Prepare archive name
ARCHIVE_NAME="gegelatilib-${VERSION_NUMBER}.$(git show -s --format="%ci" | cut -d' ' -f 1 | sed 's/-//g')$(git show -s --format="%ci" | cut -d' ' -f 2 | sed 's/://g').zip"

# Copy windows built and create an archive
cp -r ../bin/gegelatilib-${VERSION_NUMBER} ./gegelatilib-${VERSION_NUMBER}
7z a $ARCHIVE_NAME gegelatilib-${VERSION_NUMBER}/*
rm -rf gegelatilib-${VERSION_NUMBER}
  
# Go back to parent
cd ..  
