#!/bin/bash

COPYRIGHT_TEMPLATE="copyright_template.txt"
SRC_FOLDERS="./gegelatilib/ ./test/"
EXTENSION_REGEX=".*\.\(cpp\|c\|h\)"

#########
##
##  Automatically inserts the COPYRIGHT_TEMPLATE file content into all
##  *versionned** files with any of the EXTENSION_REGEX extension.
##
##  If a file already contains the copyright, it'll be removed and regenerated.
##
#########

# In windows MSYS bash, get rev command from http://gnuwin32.sourceforge.net/packages/util-linux-ng.htm

echo ""

SCRIPT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/$(basename "${BASH_SOURCE[0]}")"
SCRIPT_DIR=$(echo $SCRIPT | rev | cut -d'/' --complement -f1 | rev)

DIR=$(cd `dirname $0` && echo `git rev-parse --show-toplevel`)

cd $DIR

# Extract pattern info from copyright
PATTERN_START=$(head -1 $SCRIPT_DIR/$COPYRIGHT_TEMPLATE | sed "s/(%%DATE%%)/\\\(.+\\\)/g")
PATTERN_STOP=$(tail -1 $SCRIPT_DIR/$COPYRIGHT_TEMPLATE)

COPYRIGHT_LENGTH=$(( $(wc -l $SCRIPT_DIR/$COPYRIGHT_TEMPLATE | cut -d' ' -f1) - 1))

# Get all source files recursively
SRC_FILES=$(find ${SRC_FOLDERS} -regex ${EXTENSION_REGEX})

# Scan files one by one
for SRC_FILE in $SRC_FILES; do

	git show HEAD:"$SRC_FILE" > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		# the file is not versionned
		echo "Skip unversionned file $SRC_FILE"
		continue
	else
		echo "Process file $SRC_FILE"
	fi

	# Does the file contain the copyright
	START_LINE=$(grep --max-count=1 -rnE "$PATTERN_START" $SRC_FILE | cut -f1 -d':')
	STOP_LINE=$(grep --max-count=1 -rnE "$PATTERN_STOP" $SRC_FILE | cut -f1 -d':')

	# Count the number of matched lines
	MATCHED_LENGTH=0
	if [ "$START_LINE" != "" ] && [ "$STOP_LINE" != "" ]; then
	  MATCHED_LENGTH=$(( STOP_LINE-START_LINE ))
	fi

	# Is the pattern matched with sufficient lines
	if (( "$MATCHED_LENGTH" >= "$COPYRIGHT_LENGTH" )); then
	  # The pattern is most likely present in the file.
	  # Replace it with the template
	  TEMP_FILE=mktemp
	  head -$(( START_LINE-1 )) $SRC_FILE >> $TEMP_FILE
	  cat $SCRIPT_DIR/$COPYRIGHT_TEMPLATE | sed 's/^/ * /' | sed 's/^ \* $/ */' >> $TEMP_FILE
	  tail +$(( STOP_LINE + 1 )) $SRC_FILE >> $TEMP_FILE
	  mv $TEMP_FILE "${SRC_FILE}"

	else
	  # The pattern is judged not to be present in the file
	  # Insert it at the beginning
	  TEMP_FILE=mktemp
	  echo "/**" >> $TEMP_FILE
      cat $SCRIPT_DIR/$COPYRIGHT_TEMPLATE | sed 's/^/ * /' | sed 's/^ \* $/ */' >> $TEMP_FILE
	  printf " */\n\n" >> $TEMP_FILE
	  cat $SRC_FILE >> $TEMP_FILE
	  mv $TEMP_FILE "${SRC_FILE}"
	fi

done

# Unless specified otherwise, the rest of this script was adapted from a
# script developed by Antoine Morvan, and released under CECILL-C License.
# https://github.com/preesm/preesm/blob/master/releng/fix_header_copyright_and_authors.sh

#########
##
##  Automatically replace dates and author list
##  in files containing the corresponding patterns
##  (see {LOWDATE|UPPDATE|AUTHORS}PATTERN variable
##  below). Information is fetched from the git
##  repository.
##
##  Note: this script should be used after a pass of
##  https://wiki.eclipse.org/Development_Resources/How_to_Use_Eclipse_Copyright_Tool
##  with the correct header (see http://www.cecill.info/placer.en.html)
##  with patterns (see copyright_template.txt file) and
##  applying back the UTF-8 encoding.
##
#########
function fixFile {
    file=$1

    BASENAME=$(basename "$file")
    EXTENSION="${BASENAME##*.}"
    EXTENSION=`echo $EXTENSION | tr [a-z] [A-Z]`

	case $EXTENSION in
		BAT)
			#"@rem "
			COMMENT="@REM "
			;;
		C |	CPP | H)
			#" * "
			COMMENT=" * "
			;;
		PROPERTIES)
			#"# "
			COMMENT="# "
			;;
		XML | HTML | ECORE | GENMODEL)
			#"    "
			COMMENT="    "
			;;
		*)
			#echo "Unsupported file extension $EXTENSION"
			;;
	esac

	FILEAUTHORLISTWITHDATES=`git log --follow --use-mailmap --date=format:'%Y' --format='%ad %aN <%aE>' "$file" | sort -u`
    # Karol's Modification: remove carriage return because it is messing with grep in the for loop.
    FILEAUTHORLIST=`echo "$FILEAUTHORLISTWITHDATES" | rev | cut -d' ' -f1 | rev | sort -u | sed 's/\r//'`
    for AUTHOR in $FILEAUTHORLIST; do
		AUTHORDATELIST=`echo "$FILEAUTHORLISTWITHDATES" | grep "$AUTHOR" | cut -d' ' -f1 | sort -u`
		AUTHORUPPERDATE=`echo "$AUTHORDATELIST" | tail -n 1`
		AUTHORLOWERDATE=`echo "$AUTHORDATELIST" | head -n 1`
		if [ "$AUTHORLOWERDATE" == "$AUTHORUPPERDATE" ]; then
			AUTHORDATE="($AUTHORLOWERDATE)"
		else
			AUTHORDATE="($AUTHORLOWERDATE - $AUTHORUPPERDATE)"
		fi

		LINE=`echo "$FILEAUTHORLISTWITHDATES" | grep "$AUTHOR" | cut -d' ' -f2- | sort -u | tail -n 1`
		sed -i -e "s/$AUTHORSPATTERN/${LINE} ${AUTHORDATE}\n$COMMENT$AUTHORSPATTERN/g" "$file"
    done

	TMPFILE2=`mktemp --suffix=tosed`
	cat "$file" | grep -v "$AUTHORSPATTERN" > $TMPFILE2

	DATELIST=`echo "$FILEAUTHORLISTWITHDATES" | cut -d' ' -f1 | sort -u`
    LOWDATE=`echo "$DATELIST" | head -n 1`
    UPPDATE=`echo "$DATELIST" | tail -n 1`

	if [ "$LOWDATE" == "$UPPDATE" ]; then
		GLOBDATE="$LOWDATE"
	else
		GLOBDATE="$LOWDATE - $UPPDATE"
	fi

    cat "$TMPFILE2" | sed -e "s/$DATEPATTERN/$GLOBDATE/g" > "$file"
    rm $TMPFILE2
}

# from https://github.com/fearside/ProgressBar/
function ProgressBar {
    let _progress=(${1}*100/${2}*100)/100
    let _done=(${_progress}*5)/10
    let _left=50-$_done
    _fill=$(printf "%${_done}s")
    _empty=$(printf "%${_left}s")
	printf "\rProgress : [${_fill// /#}${_empty// /-}] ${_progress}%%"
}

DATEPATTERN="%%DATE%%"
AUTHORSPATTERN="%%AUTHORS%%"

TMPFILE=`mktemp --suffix=biglisttosed`
grep "%%AUTHORS%%" -R | cut -d':' -f1 | sort -u | grep -v "copyright_template.txt" | grep -v "fix_header_copyright_and_authors.sh" | grep -v "copyright_identification.sh" | grep -v "README" > $TMPFILE

echo $TMPFILE
cat $TMPFILE

echo ""
echo " Header template applied."
echo " Replacing author and date patterns using git log data..."
echo ""

NBFILES=`cat $TMPFILE | wc -l`
NBFILESPROCESSED=0
time (
NBCPUS=`grep -c ^processor /proc/cpuinfo`
((NBTHREADS=NBCPUS*2))
while read -r line
do
  MOD=$((NBFILESPROCESSED % NBTHREADS))
  [ $MOD -eq 0 ] && ProgressBar ${NBFILESPROCESSED} ${NBFILES} && wait
  NBFILESPROCESSED=$((NBFILESPROCESSED+1))
  fixFile "$line" &
done < $TMPFILE
ProgressBar $((NBFILES-1)) ${NBFILES}
echo " Done."
wait
)
rm $TMPFILE

echo ""