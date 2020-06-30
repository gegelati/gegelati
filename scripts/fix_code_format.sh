#!/bin/bash

SRC_FOLDERS="./gegelatilib/ ./test/"
EXTENSION_REGEX=".*\.\(cpp\|c\|h\)"
COMMIT_AUTHOR="Vaader Bot <vaader-bot@insa-rennes.fr>"

#########
##
## Automatically calls `clang-format` on all files
##
## The `--doCommit` option can be given to the script to commit the code 
## reformatting that was done (if any).
##
## The `--keepChanges` option can be given to keep the code formatting done
## by the script without committing them. 
##
## Without these option, the script will undo its work, and exit with an error 
## if any code file was modified by the formatting tool. If not file was 
## modified the script will exit normally.
##
#########

echo ""

DO_COMMIT=false
KEEP_CHANGES=false
# Maximum number of times clang-format will be called to try to reach a fixpoint.
MAX_NB_ITERATION=10


optspec=":-:"
while getopts "$optspec" optchar; do
case "${OPTARG}" in
   doCommit)
      echo "Code formatting will be committed."
      DO_COMMIT=true
	  break;
      ;;
	keepChanges)
      echo "Code formatting will be kept, but not committed."
	  KEEP_CHANGES=true
	;;
   *)
      echo "Option \"$OPTARG\" not recognized."
      exit 1
      ;;
esac
done

if [ "$CLANG_FORMAT" = "" ]; then CLANG_FORMAT=clang-format; fi
$CLANG_FORMAT -version

#Check for modification in versionned files
git update-index -q --refresh
git diff-index --quiet HEAD -- || printf "Some versionned files have untracked modification.\nCommit, stash or reset before launching this script.\nScript exiting.";
git diff-index --quiet HEAD -- || exit;

# Get all source files recursively
SRC_FILES=$(find ${SRC_FOLDERS} -regex ${EXTENSION_REGEX})

function format_files() {
	# Scan files one by one
	for SRC_FILE in $SRC_FILES; do
		echo "Format $SRC_FILE."
		$CLANG_FORMAT -i --style=file $SRC_FILE
	done
}

format_files

# Were any change made?
git update-index -q --refresh
CHANGE_MADE=false
git diff-index --quiet HEAD -- || CHANGE_MADE=true;

echo ""
if [ "$CHANGE_MADE" = true ] && [ "$DO_COMMIT" = true ]; then
	# Commit changes
	echo "Committing changes"
	git add -u
	git -c user.name="$(echo $COMMIT_AUTHOR | cut -d' ' -f1,2)" -c user.email="$(echo $COMMIT_AUTHOR | cut -d'<' -f2 | cut -d'>' -f1)" commit -m "(Releng) Format source code."
	
	# Try to reach a fixpoint
	for ((i=1; i<=$MAX_NB_ITERATION ; i++))
	do
		echo "Try ${i}: Checking for a fixpoint."
		format_files
		CHANGE_MADE=false
		git update-index -q --refresh
		git diff-index --quiet HEAD -- || CHANGE_MADE=true;
		if [ "$CHANGE_MADE" = true ]; then
			echo "Stable format not reached."
			git add -u
			git commit --amend --no-edit
		else
			echo "Stable format reached."
			break;
		fi
	done
	if [[ $i -gt $MAX_NB_ITERATION ]]; then
		echo "Error: Stable format not reached after $MAX_NB_ITERATION tries."
		echo "Reverting all changes."
		git reset --hard HEAD~1
		exit 1;
	fi
elif [ "$CHANGE_MADE" = true ] && [ "$KEEP_CHANGES" = false ]; then
	# Error
	echo "Code is not properly formatted."
	echo "Use fix_code_format.sh -doCommit to commit a proper code formatting."
	echo "Reverting all changes."
	git reset --hard
	exit 1;
elif [ "$CHANGE_MADE" = true ] && [ "$KEEP_CHANGES" = true ]; then
	# Keeping the changes
	echo "Code has been reformatted."
	echo "Changes are kepts but not comitted."
	exit;
else
	echo "All files are already properly formatted."
fi

echo ""