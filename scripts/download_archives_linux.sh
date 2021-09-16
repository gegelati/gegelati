#!/bin/bash
set -x

TARGET_GH_REPO=gegelati/neutral-builds
SITE_GITHUB_BRANCH=gh-pages
NEW_SITE_FOLDER=neutral_builds
MAX_VERSIONS_COUNT=10
SITE_NAME="GEGELATI - Neutral Builds"
RELEASE_NOTES_FILE="release_notes.md"

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


# Prepare index
echo "# GEGELATI Neutral Builds 

<table>
<tr><td><b>Date</b></td><td><b>Time</b></td><td><b>Commit</b></td><td><b>Windows</b></td></tr>" > index.md  

# And readme
echo "# GEGELATI Neutral Builds 
|Date|Time|Commit|Windows|
|----|----|------|-------|" > ReadMe.md  

# Generate list
ARCHIVES=$(find -maxdepth 1 -regextype posix-egrep -regex "./gegelatilib-[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\.zip\$" | rev | cut -d'/' -f 1 | rev | sort -t. -k 1,1nr -k 2,2nr -k 3,3nr -k 4,4nr | head -n $MAX_VERSIONS_COUNT)

count=0
for archive in ${ARCHIVES}; do
	# Variables
	commit_date=$(git show -s --format="%ci" HEAD~$count| cut -d' ' -f 1 | cut -d'-' -f 1,2,3 --output-delimiter='.')
	commit_time=$(git show -s --format="%ci" HEAD~$count| cut -d' ' -f 2)
	commit_short_sha1=$(git show -s --format="%h" HEAD~$count)
	commit_long_sha1=$(git show -s --format="%H" HEAD~$count)
	archive_size=$(ls -s --block-size=K ./$archive | cut -d' ' -f 1)

	if [ $count = 0 ] 
	then
		cp $archive gegelatilib-latest-develop.zip
		cp $archive ../$(echo $archive | cut -d'.' -f 1,2,3,5) # Copy for releases drop the date

		# index.md html table
		echo "<tr><td colspan='2'><div align='center'><i>Latest</i></div></td><td><a href=\"https://github.com/gegelati/gegelati/commit/$commit_long_sha1\"><code>$commit_short_sha1</code></a></td><td><a href=\"./gegelatilib-latest-develop.zip\">Zip ($archive_size)</a></td></tr>" >> index.md
		
		# readme.md markdown table
		echo "| Latest |  | [\`$commit_short_sha1\`](https://github.com/gegelati/gegelati/commit/$commit_long_sha1) | [Zip ($archive_size)](./gegelatilib-latest-develop.zip) |" >> ReadMe.md 

	fi	

	# index.md html table
	echo "<tr><td>$commit_date</td><td>$commit_time</td><td><a href=\"https://github.com/gegelati/gegelati/commit/$commit_long_sha1\"><code>$commit_short_sha1</code></a></td><td><a href=\"./$archive\">Zip ($archive_size)</a></td></tr>" >> index.md

	# readme.md markdown table
	echo "| $commit_date | $commit_time | [\`$commit_short_sha1\`](https://github.com/gegelati/gegelati/commit/$commit_long_sha1) | [Zip ($archive_size)](./${archive}) |" >> ReadMe.md 
	count=$((count+1))
done

echo "</table>" >> index.md

# Go back to parent
cd ..  

# Prepare Release Notes in case this is a release.
rm -rf $RELEASE_NOTES_FILE

found=0;
while read -r line
do
  match=$(echo "$line" | sed -E -e 's/## Release version [0-9]+.[0-9]+.[0-9]+/1/')
  if [ $found = 0 ]
  then
	if [ "$match" = "1" ]
	then
		found=1
		echo "$line" >> $RELEASE_NOTES_FILE
	fi
  else
    if [ "$match" = "1" ]
	then
		break
	else
	  echo "$line" >> $RELEASE_NOTES_FILE
	fi
  fi
done < "Changelog.md"
