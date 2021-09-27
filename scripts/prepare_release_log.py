
import re

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
            found = True
            releaseNoteFile.write(line)
    else:
        if match:
            break
        else:
            releaseNoteFile.write(line)

releaseNoteFile.close()