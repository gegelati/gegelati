#!/bin/bash
set -x

# Prepare folder
mkdir neutral_builds
cd neutral_builds

# Get archive from windows (assume success)
curl -L -O -J https://gegelati.shortcm.li/windows-package

# Prepare readme
# Create Markdown index
echo "# GEGELATI Neutral Builds 
* Windows: 
  * [Latest](./gegelatilib-0.0.0.zip) for more information." > index.md  

# Go back to parent
cd ..  