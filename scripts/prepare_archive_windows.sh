#!/bin/bash

# Prepare folder
mkdir neutral_builds
cd neutral_builds

# Copy windows built and create an archive
cp -r ../bin/gegelatilib-0.0.0 ./gegelatilib-0.0.0
7z a gegelatilib-0.0.0.zip gegelatilib-0.0.0/*
rm -rf gegelatilib-0.0.0
  
# Go back to parent
cd ..  