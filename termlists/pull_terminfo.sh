#!/usr/bin/env bash
#############################################################################
# Download terminfo.src
########################################
TODAY=`date +"%Y%m%d"`
#MASTER="https://invisible-mirror.net/archives/ncurses/current/terminfo.src.gz"
#MASTER="https://invisible-island.net/datafiles/current/terminfo.src.gz"
MASTER="https://ncurses.scripts.mit.edu/?p=ncurses.git;a=blob_plain;f=misc/terminfo.src;hb=HEAD"

# MASTER FILE HEADER FIRST LINE MUST INCLUDE
MEXPECT='TERMINAL TYPE DESCRIPTIONS'

TODAYDL="${TODAY}-terminfo.src.dl"
TODAYFILE="${TODAY}-terminfo.src"
LATESTLINK="terminfo.src.latest"

# Options
DEBUG=0  # -d | --debug
KEEP=0  # --keep

eprintf()
{
    printf "$@" 1>&2
}

pdebug()
{
    if [ "1" = "$DEBUG" ]
    then
        OUT=`printf "$@"` # Assignment drops newlines (if they existed)
        eprintf "DEBUG: %s\n" "$OUT"
    fi
}

for A in "$@"
do
    if [ "$A" = "--keep" ]
    then
        KEEP="1"
        pdebug "Turned KEEP on\n"
    elif [ "$A" = "--force" ]
    then
        # --force
        #     This skips non-fatal checks on existing files.
        #     curl will be forced to download a fresh copy.
        #     BUT if $TODAYDL exists, curl will NOT download again.
        FORCE="1"
        pdebug "Turned FORCE on\n"
    elif [ "$A" = "--debug" -o "$A" = "-d" ]
    then
        DEBUG="1"
        pdebug "Turned DEBUG on\n"
    fi
done

# Pre-checks
if [ -e "$TODAYDL" -a "1" != "$FORCE" ]
then
    eprintf "ERROR: '%s' exists, did this get interrupted?\n" "$TODAYDL"
    eprintf "ERROR: Move or remove '%s' and run again?\n" "$TODAYDL"
    exit 1
elif [ -e "$TODAYDL" ]
then
    pdebug "Force skipped bailout: '%s' exists" "$TODAYDL"
fi
if [ -s "$LATESTLINK" ]
then
    :
elif [ -e "$LATESTLINK" ]
then
    eprintf "ERROR: Non-softlink '%s', exists.\n" "$LATESTLINK"
    eprintf "ERROR: Move or remove '%s' and run again?\n" "$LATESTLINK"
    exit 1
fi
if [ -f "${TODAYFILE}" -a "1" != "$FORCE" ]
then
    eprintf "INFO: '%s' already exists.\n" "${TODAYFILE}"
    ln -sf "${TODAYFILE}" "terminfo.src.latest"
    exit 0
elif [ -e "$TODAYFILE" ]
then
    pdebug "Force skipped bailout: '%s' exists" "$TODAYFILE"
fi

########################################
# The ACTUAL Download!
########################################
if [ ! -f "$TODAYDL" ]
then
    pdebug 'curl -L "%s" -o "%s"' "$MASTER" "${TODAYDL}"
    curl -L "$MASTER" -o "${TODAYDL}"
else
    pdebug 'SKIPPING RE-DOWNLOAD: "%s" -> "%s"' "$MASTER" "${TODAYDL}"
fi
if [ -f "${TODAYDL}" ]
then
    # Downloaded File Exists...
    # Check header for correct info
    FIRSTLINE=`head -1 "${TODAYDL}"`
    echo "$FIRSTLINE" | grep "$MEXPECT" >/dev/null 2>&1
    if [ "0" != "$?" ]
    then
        eprintf "ERROR: Sanity check on '%s' header failed.\n" "${TODAYDL}"
        pdebug "EXPECTING: '%s'\n" "$MEXPECT"
        pdebug "IN LINe: '%s'\n" "$FIRSTLINE"
        if [ "1" != "$KEEP" ]
        then
            rm -f "${TODAY}-terminfo.src.dl"
        fi
        exit 1
    fi
    LINECOUNT=`wc -l "${TODAYDL}" | awk '{print $1}'`
    if [ 20000 -ge $LINECOUNT ]
    then
        eprintf "ERROR: File '%s' too short at '%s' lines.  " \
            "${TODAYDL}" "${LINECOUNT}"
        eprintf "Expecting over 20000 lines.\n"
        if [ "1" != "$KEEP" ]
        then
            rm -f "${TODAY}-terminfo.src.dl"
        fi
        exit 1
    fi
    pdebug 'Moving "%s" -> "%s"' "${TODAYDL}" "${TODAYFILE}"
    mv "${TODAYDL}" "${TODAYFILE}"
    pdebug 'Softlinking (-sf) "%s" -> "%s"' \
        "${TODAYFILE}" "terminfo.src.latest"
    ln -sf "${TODAYFILE}" "terminfo.src.latest"
else
    # If curl itself didn't create the download file...
    eprintf "ERR: Curl unable to download '%s'.\n" "${TODAYFILE}"
    exit 1
fi
