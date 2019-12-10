#!/bin/bash

# Go to neutral build folder
cd neutral_builds

# Uploads the built library onto transfer.sh
PACKAGE_LOCATION="$(curl --upload-file gegelatilib-0.0.0.zip https://transfer.sh/gegegelatilib-0.0.0.zip)"

# Update url with short.cm
curl --request POST --url https://api.short.cm/links/240039262 --header 'accept: application/json' --header 'authorization: nDpuKUpkjU6Zgpwc' --header 'content-type: application/json' --data '{"domain":"gegelati.shortcm.li","originalURL":"'$PACKAGE_LOCATION'","allowDuplicates":false}'

# Back to parent folder
cd ..
