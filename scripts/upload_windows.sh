#!/bin/bash
set -x

# Get version number from CMakeLists.txt
VERSION_NUMBER="$(cat CMakeLists.txt | grep -m 1 "VERSION \([0-9]\+.[0-9]\+.[0-9]\+\)" | sed -E -e 's/ |\)|VERSION//g')"

# Go to neutral build folder
cd neutral_builds

# Recreate archive name
ARCHIVE_NAME="gegelatilib-${VERSION_NUMBER}.$(git show -s --format="%ci" | cut -d' ' -f 1 | sed 's/-//g')$(git show -s --format="%ci" | cut -d' ' -f 2 | sed 's/://g').zip"

# Uploads the built library onto transfer.sh
PACKAGE_LOCATION=$(curl -F"file=@${ARCHIVE_NAME}" https://file.io)

# Check upload
if [[ $PACKAGE_LOCATION =~ \"success\":true,.*\"link\":\"(.+)\",.* ]]
then
	LINK="${BASH_REMATCH[1]}"
else
	exit 255
fi

# Update url with short.cm
curl --request POST --url https://api.short.cm/links/240039262 --header 'accept: application/json' --header 'authorization: nDpuKUpkjU6Zgpwc' --header 'content-type: application/json' --data '{"domain":"gegelati.shortcm.li","originalURL":"'$LINK'","allowDuplicates":false}'

# Back to parent folder
cd ..

exit 0
