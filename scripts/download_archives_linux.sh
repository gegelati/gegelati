#!/bin/bash
set -x

TARGET_GH_REPO=gegelati/neutral-builds
SITE_GITHUB_BRANCH=gh-pages
NEW_SITE_FOLDER=neutral_builds
MAX_VERSIONS_COUNT=10
SITE_NAME="GEGELATI - Neutral Builds"

## -- fetch current site
CURRENT_SITE=current
rm -rf ${CURRENT_SITE}
git clone https://github.com/${TARGET_GH_REPO}.git -b ${SITE_GITHUB_BRANCH} ${CURRENT_SITE}

CURRENT_PUBLISHED_RELEASES=$(find ${CURRENT_SITE} -maxdepth 1 -regextype posix-egrep -regex "${CURRENT_SITE}/gegelatilib-[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\.zip\$" | rev | cut -d'/' -f 1 | rev | sort -t. -k 1,1n -k 2,2n -k 3,3n -k 4,4n | tail -n $((MAX_VERSIONS_COUNT-1)) )

# Prepare folder
rm -rf ${NEW_SITE_FOLDER}
mkdir ${NEW_SITE_FOLDER}

# Copy old archives
for old_release in ${CURRENT_PUBLISHED_RELEASES}; do
  cp -R ${CURRENT_SITE}/${old_release} ${NEW_SITE_FOLDER}/${old_release}
done

# Go into folder
cd ${NEW_SITE_FOLDER}

# Get latest archive from windows 
curl -L -O -J https://gegelati.shortcm.li/windows-package

# Prepare readme
echo "# GEGELATI Neutral Builds 
* Windows:" > index.md  

# Generate list
ARCHIVES=$(find -maxdepth 1 -regextype posix-egrep -regex "./gegelatilib-[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\.zip\$" | rev | cut -d'/' -f 1 | rev | sort -t. -k 1,1n -k 2,2n -k 3,3n -k 4,4n | tail -n $MAX_VERSIONS_COUNT)

for archive in ${ARCHIVES}; do
	echo "\n  * [LABEL](./${archive})" >> index.md 
done


# Go back to parent
cd ..  
