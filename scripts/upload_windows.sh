#!/bin/bash
set -x

# Go to neutral build folder
cd neutral_builds

# Recreate archive name
ARCHIVE_NAME="gegelatilib-0.0.0.$(git show -s --format="%ci" | cut -d' ' -f 1 | sed 's/-//g')$(git show -s --format="%ci" | cut -d' ' -f 2 | sed 's/://g').zip"

# Uploads the built library onto transfer.sh
PACKAGE_LOCATION="$(curl -m 600 --upload-file $ARCHIVE_NAME https://transfer.sh/$ARCHIVE_NAME)"

# Check upload
if [ "$PACKAGE_LOCATION" = "" ]
then
	exit 255
fi

# Update url with short.cm
curl --request POST --url https://api.short.cm/links/240039262 --header 'accept: application/json' --header 'authorization: nDpuKUpkjU6Zgpwc' --header 'content-type: application/json' --data '{"domain":"gegelati.shortcm.li","originalURL":"'$PACKAGE_LOCATION'","allowDuplicates":false}'

# Back to parent folder
cd ..

exit 0
