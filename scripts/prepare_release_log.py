# This script prepares the release note for the latest version from the change log.
#
# Author: K. Desnos
# License: CeCILL-C

import re
import os
import shutil

CHANGELOG_FILE = "Changelog.md"
RELEASE_NOTES_FILE="release_notes.md"

changelogFile = open("{}".format(CHANGELOG_FILE),"r")
releaseNoteFile = open("{}".format(RELEASE_NOTES_FILE),"w")

# Read the file line by line until the first release notes are completed.
found = False
while True:
    # Read one line
    line = changelogFile.readline()

    # Is it starting a new release note?
    match = re.match(r'## Release version [0-9]+\.[0-9]+\.[0-9]+',line)

    if not found:
        if match:
            # First line of release note found
            found = True
            releaseNoteFile.write(line)

            # Prepare release archive
            release_name_msvc = re.sub(r'## Release version ([0-9]+\.[0-9]+\.[0-9]+).*\n', r'gegelatilib-msvc-\1.zip', line)
            release_name_mingw = re.sub(r'## Release version ([0-9]+\.[0-9]+\.[0-9]+).*\n', r'gegelatilib-mingw-\1.zip', line)

            # Get file list of old builds from current site
            def glob_re(pattern, folder):
                return list(filter(re.compile(pattern).match, os.listdir(folder)))

            archive_msvc = glob_re(r'gegelatilib-msvc.*.zip', r'neutral_builds')[0]
            shutil.move(r'neutral_builds/{}'.format(archive_msvc), r'neutral_builds/{}'.format(release_name_msvc))
            
            archive_mingw = glob_re(r'gegelatilib-mingw.*.zip', r'neutral_builds')[0]
            shutil.move(r'neutral_builds/{}'.format(archive_mingw), r'neutral_builds/{}'.format(release_name_mingw))

    else:
        if match:
            break
        else:
            releaseNoteFile.write(line)

releaseNoteFile.close()