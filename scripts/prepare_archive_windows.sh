#!/bin/bash
set -x

# Prepare folder
mkdir neutral_builds
cd neutral_builds


# Prepare archive name
ARCHIVE_NAME="gegelatilib-0.0.0.$(git show -s --format="%ci" | cut -d' ' -f 1 | sed 's/-//g')$(git show -s --format="%ci" | cut -d' ' -f 2 | sed 's/://g').zip"

# Copy windows built and create an archive
cp -r ../bin/gegelatilib-0.0.0 ./gegelatilib-0.0.0
7z a $ARCHIVE_NAME gegelatilib-0.0.0/*
rm -rf gegelatilib-0.0.0
  
# Go back to parent
cd ..  
