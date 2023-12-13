# This script prepares the neutral build github page based on
# - Previous neutral builds on the page
# - Latest build produced by the CI.
#
# The neutral build page is produced both as a markdown and an html page.
#
# Author: K. Desnos
# License: CeCILL-C

import os
import shutil
import stat
import re
import subprocess

TARGET_GH_REPO="gegelati/neutral-builds"
SITE_GITHUB_BRANCH="gh-pages"
NEW_SITE_FOLDER="neutral_builds"
MAX_VERSIONS_COUNT=10
SITE_NAME="GEGELATI - Neutral Builds"
CURRENT_SITE="current"

# Fetch current site
# Function used to delete .git hidden file.
def del_rw(action, name, exc):
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

# Clone the TARGET_GH_REPO repository on github.com
os.system("git clone https://github.com/{}.git -b {} {}".format(TARGET_GH_REPO, SITE_GITHUB_BRANCH, CURRENT_SITE))

# Get file list of old builds from current site
def glob_re(pattern, folder):
    return list(filter(re.compile(pattern).match, os.listdir(folder)))

files_msvc  = glob_re(r'gegelatilib(-msvc)?-([0-9]+\.)+zip', CURRENT_SITE)
files_mingw = glob_re(r'gegelatilib-mingw-([0-9]+\.)+zip', CURRENT_SITE)

# Remove oldest element from the list to keep only MAX_VERSION_COUNT versions
if len(files_msvc) > (MAX_VERSIONS_COUNT - 1):
    del files_msvc[0:(len(files_msvc) - MAX_VERSIONS_COUNT + 1)]

if len(files_mingw) > (MAX_VERSIONS_COUNT - 1):
    del files_mingw[0:(len(files_mingw) - MAX_VERSIONS_COUNT + 1)]

# Latest archive is already in the NEW_SITE_FOLDER
latest_file_msvc = glob_re(r'gegelatilib-msvc-([0-9]+\.)+zip', NEW_SITE_FOLDER)[0]
latest_file_mingw = glob_re(r'gegelatilib-mingw-([0-9]+\.)+zip', NEW_SITE_FOLDER)[0]

# Copy old builds
for file in files_msvc:
    shutil.copyfile("{}/{}".format(CURRENT_SITE,file), "{}/{}".format(NEW_SITE_FOLDER,file))
for file in files_mingw:
    shutil.copyfile("{}/{}".format(CURRENT_SITE,file), "{}/{}".format(NEW_SITE_FOLDER,file))

# Add latest file to the list
files_msvc.append(latest_file_msvc)
files_mingw.append(latest_file_mingw)

# Sort files alphabetically (now sorted from newest to oldest)
files_msvc.sort(reverse= True)
files_mingw.sort(reverse= True)

# Create the markdown files
indexFile = open("{}/index.md".format(NEW_SITE_FOLDER),"w")
readmeFile = open("{}/ReadMe.md".format(NEW_SITE_FOLDER),"w")

indexFile.write("# GEGELATI Neutral Builds\n\n"
             "<table>\n"
             "<tr><td><b>Date</b></td><td><b>Time</b></td><td><b>Commit</b></td><td><b>MSVC</b></td><td><b>MinGW</b></td></tr>\n")
readmeFile.write("# GEGELATI Neutral Builds\n"
                 "|Date|Time|Commit|MSVC|MinGW|\n"
                 "|----|----|------|----|-----|\n")

# Generate files content, version by version
count = 0
for file in files_msvc:
    # Get Git infos
    commit_date_time = subprocess.check_output("git show -s --format=\"%ci\" HEAD~{}".format(count), shell=True)
    commit_date_time = commit_date_time.decode("utf-8").strip()
    commit_date = re.sub(r'([0-9]+-[0-9]+-[0-9]+).*',r'\1',commit_date_time)
    commit_time = re.sub(r'.* ([0-9]+:[0-9]+:[0-9]+).*',r'\1',commit_date_time)

    commit_short_sha1 = subprocess.check_output("git show -s --format=\"%h\" HEAD~{}".format(count), shell=True)
    commit_short_sha1 = commit_short_sha1.decode("utf-8").strip()
    commit_long_sha1 = subprocess.check_output("git show -s --format=\"%H\" HEAD~{}".format(count), shell=True)
    commit_long_sha1 = commit_long_sha1.decode("utf-8").strip()

    file_size_msvc = os.path.getsize("{}/{}".format(NEW_SITE_FOLDER, file)) / 1024
    file_size_msvc = "{}K".format(int(file_size_msvc))

    if(len(files_mingw) > count):
        file_size_mingw = os.path.getsize("{}/{}".format(NEW_SITE_FOLDER, files_mingw[count])) / 1024
        file_size_mingw = "{}K".format(int(file_size_mingw))

    # Prepare latest build and release file.
    if count == 0:
        shutil.copyfile("{}/{}".format(NEW_SITE_FOLDER,file),"{}/gegelatilib-msvc-latest-develop.zip".format(NEW_SITE_FOLDER))
        shutil.copyfile("{}/{}".format(NEW_SITE_FOLDER,files_mingw[count]),"{}/gegelatilib-mingw-latest-develop.zip".format(NEW_SITE_FOLDER))

        # Write entry into markdown files
        indexFile.write("<tr><td colspan='2'><div align='center'><i>Latest</i></div></td><td><a href=\"https://github.com/gegelati/gegelati/commit/{}\"><code>{}</code></a></td><td><a href=\"./gegelatilib-msvc-latest-develop.zip\">Zip ({})</a></td><td><a href=\"./gegelatilib-mingw-latest-develop.zip\">Zip ({})</a></td></tr>\n".format(commit_long_sha1, commit_short_sha1, file_size_msvc, file_size_mingw))
        readmeFile.write("| Latest |  | [\`{}\`](https://github.com/gegelati/gegelati/commit/{}) | [Zip ({})](./gegelatilib-msvc-latest-develop.zip) |[Zip ({})](./gegelatilib-mingw-latest-develop.zip) |\n".format(commit_short_sha1, commit_long_sha1, file_size_msvc, file_size_mingw))

    # Write normal entry into markdown
    indexFile.write("<tr><td>{}</td><td>{}</td><td><a href=\"https://github.com/gegelati/gegelati/commit/{}\"><code>{}</code></a></td><td><a href=\"./{}\">Zip ({})</a></td>".format(commit_date, commit_time, commit_long_sha1, commit_short_sha1, file, file_size_msvc))
    readmeFile.write("| {} | {} | [\`{}\`](https://github.com/gegelati/gegelati/commit/{}) | [Zip ({})](./{}) |".format(commit_date, commit_time, commit_short_sha1, commit_long_sha1, file_size_msvc, file))

    if(len(files_mingw) > count):
        indexFile.write("<td><a href=\"./{}\">Zip ({})</a></td>".format(files_mingw[count], file_size_mingw))
        readmeFile.write("[Zip ({})](./{}) |".format(file_size_mingw, files_mingw[count]))
    else:
        indexFile.write("<td></td>")
        readmeFile.write("|")

    indexFile.write("</tr>\n")
    readmeFile.write("\n")
    # increase counter
    count+=1

# End html table
indexFile.write("</table>\n")

# Close files
indexFile.close()
readmeFile.close()