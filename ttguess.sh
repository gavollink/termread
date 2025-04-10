#!/usr/bin/env sh
#############################################################################
export SAVETERM="${TERM}"

__debug_p ()
{
    if [ -n "$DEBUG" ]
    then
        __e_setecho -b
        "$_ECHO" $* >&2
    fi
}

########################################
# -b     Optional, attempt built-in echo OK if system echo cannot be found.
# Sets GLOBALS:
#  _ECHO
__e_setecho ()
{
    MUSTSYS=1
    if [ -n "$1" -a "$1" = "-b" ]
    then
        shift
        MUSTSYS=0
    fi
    if [ -z "${_ECHO}" -o ! -x "${_ECHO}" ]
    then
        if [ -x /usr/local/bin/echo ]; then
            _ECHO=/usr/local/bin/echo
        elif [ -x /usr/bin/echo ]; then
            _ECHO=/usr/bin/echo
        elif [ -x /bin/echo ]; then
            _ECHO=/bin/echo
        else
            if [ "1" = "${MUSTSYS}" ]
            then
                echo "ERROR: Unable to find system tool, echo." >&2
                return 1
            else
                _ECHO=echo
            fi
        fi
        export _ECHO
    fi
    return 0
}

########################################
# -b     Optional, attempt built-in OK if system ver cannot be found.
# $ARG1  system command to search for.
# $ARG2  ENV VAR to put path to system command.
##
# Sets GLOBALS:
#     _ECHO      (calls __e_setecho)
#     $ARG2
__e_setsyscmd ()
{
    if ! __e_setecho -b; then return 1; fi
    MUSTSYS=1
    if [ ! -z "$1" -a "$1" = "-b" ]
    then
        shift
        MUSTSYS=0
    fi
    WANTCMD=$1
    WANTENV=$2
    eval 'TWANT="${'`${_ECHO} ${WANTENV}`'}"'
    if [ -z "${TWANT}" -o ! -x "${TWANT}" ]
    then
        if [ -x   "/usr/local/X11/bin/${WANTCMD}" ]; then
            TWANT="/usr/local/X11/bin/${WANTCMD}"
        elif [ -x "/usr/local/sbin/${WANTCMD}" ]; then
            TWANT="/usr/local/sbin/${WANTCMD}"
        elif [ -x "/usr/local/bin/${WANTCMD}" ]; then
            TWANT="/usr/local/bin/${WANTCMD}"
        elif [ -x "/usr/X11/bin/${WANTCMD}" ]; then
            TWANT="/usr/X11/bin/${WANTCMD}"
        elif [ -x "/opt/X11/bin/${WANTCMD}" ]; then
            TWANT="/opt/X11/bin/${WANTCMD}"
        elif [ -x "/usr/xpg4/bin/${WANTCMD}" ]; then
            TWANT="/usr/xpg4/bin/${WANTCMD}"      # Solaris, sometimes
        elif [ -x "/usr/xpg4/${WANTCMD}" ]; then
            TWANT="/usr/xpg4/${WANTCMD}"          # Solaris, sometimes
        elif [ -x "/usr/ucb/bin/${WANTCMD}" ]; then
            TWANT="/usr/ucb/bin/${WANTCMD}"       # Solaris, sometimes
        elif [ -x "/usr/ucb/${WANTCMD}" ]; then
            TWANT="/usr/ucb/${WANTCMD}"           # Solaris, sometimes
        elif [ -x "/usr/sbin/${WANTCMD}" ]; then
            TWANT="/usr/sbin/${WANTCMD}"
        elif [ -x "/usr/bin/${WANTCMD}" ]; then
            TWANT="/usr/bin/${WANTCMD}"
        elif [ -x "/sbin/${WANTCMD}" ]; then
            TWANT="/sbin/${WANTCMD}"
        elif [ -x "/bin/${WANTCMD}" ]; then
            TWANT="/bin/${WANTCMD}"
        else
            _TEST=`which ${WANTCMD}`
            if [ -n "${_TEST}" -a -x "${_TEST}" ]
            then
                TWANT="$_TEST"
            elif [ "1" = "${MUSTSYS}" ]
            then
                ${_ECHO} "ERROR: Unable to find system tool, ${WANTCMD}." >&2
                return 1
            else
                TWANT=${WANTCMD}
            fi
        fi
        eval ${WANTENV}'="'${TWANT}'"'
        export $WANTENV
    fi
    unset EVALUATE
    unset MUSTSYS
    unset WANTCMD
    unset WANTENV
    unset TWANT
    return 0
}

__set_term_info_cx()
{
    local INFO=$1; shift
    local TRYLIST=$@

    __set_cterm_fallback $TRYLIST
    if [ "0" = "$?" ]; then
        _TERMSET=1
    fi
    if [ -n "$INFO" ]
    then
        _TM_EMOJI=$(echo $INFO | awk -F: '{print $1}'); export _TM_EMOJI
        _TM_COLORS=$(echo $INFO | awk -F: '{print $2}'); export _TM_COLORS
        _TM_TRUECLR=$(echo $INFO | awk -F: '{print $3}'); export _TM_TRUECLR
        if [ "1" == "$_TM_TRUECOLOR" ]
        then
            _TM_TRUEMODE=$(echo $INFO | awk -F: '{print $4}')
            export _TM_TRUEMODE
            COLORTERM=truecolor; export COLORTERM
        fi
    fi
}

__set_term_info_x()
{
    local INFO=$1; shift
    local TRYLIST=$@

    __set_term_fallback_x $TRYLIST
    if [ "0" = "$?" ]; then
        _TERMSET=1
    fi
    if [ -n "$INFO" ]
    then
        _TM_EMOJI=$(echo $INFO | awk -F: '{print $1}'); export _TM_EMOJI
        _TM_COLORS=$(echo $INFO | awk -F: '{print $2}'); export _TM_COLORS
        _TM_TRUECLR=$(echo $INFO | awk -F: '{print $3}'); export _TM_TRUECLR
        if [ "1" = "$_TM_TRUECLR" ]
        then
            _TM_TRUEMODE=$(echo $INFO | awk -F: '{print $4}')
            export _TM_TRUEMODE
            COLORTERM=truecolor; export COLORTERM
        fi
    fi
}

_known_terminal()
{
    local KNOW=$1
              # EMOJI_SUPPORT:COLOR_INDEX_MAX:TRUECOLOR[:TRUECOLOR_MODE]
    local INFO=""
    _TM_KITTY=0;  export _TM_KITTY
    _TM_ITERM2=0; export _TM_ITERM2
    case "$KNOW" in
        putty)
            # TRUECLR verified on Windows PuTTY 0.76 (colon ignored)
            __set_term_info_x \
                "1:256:1:semi" \
                "putty-256color" "putty" "xterm-256color"
            _TM_PUTTY=1;       export _TM_PUTTY
            ;;
        rxvt-unicode-256color)
            __set_term_info_x \
                "0:256:1:colon" rxvt-unicode-256color xterm-256color
            ;;
        vt102)
            ####
            ## THIS IS A BAD CALL, HONESTLY
            ## Nothing in the vt100 range originally responded to
            ## Secondary DA (it didn't exist yet), but
            ## every other terminal emulator gives us SOME
            ## other response.
            ####
            if [ "st" != "$TERM" ]; then
                __set_term_info_x "0:8:0" linux vt102
                if [ "0" = "$?" ]; then
                    _TERMSET=1
                fi
            else
                _TM_EMOJI=0; export _TM_EMOJI
                _TM_COLORS=8; export _TM_COLORS
                _TM_TRUECLR=0; export _TM_TRUECLR
                _TERMSET=1
            fi
            ;;
        JediTerm)
            __set_term_info_x "0:256:0" jedi xterm-256color
            ;;
        vte)
            __set_term_info_x "1:256:1:colon" \
                vte-256color gnome-256color vte gnome xterm-256color
            ;;
        alacritty)
            __set_term_info_x "1:256:1:colon" \
                alacritty rio xterm-256color
            ;;
        msterm18)
            __set_term_info_x "1:256:0" \
                ms-terminal ms-vt-utf8 ms-vt100+ \
                ms-vt100-color xterm-256color
            ;;
        vt101)
            __set_term_info_x "0:2:0" vt101
            ;;
        vt100)
            __set_term_info_x "0:2:0" vt100-basic vt100
            ;;
        byobu)
            _set_term_info_c "1:256:0" xterm
            ;;
        konsole)
            __set_term_info_x "1:256:1:semi" \
                konsole-256color konsole xterm-256color
            ;;
        konsole-old)
            __set_term_info_x "0:256:0" \
                konsole-256color konsole xterm-256color
            ;;
        terminal.app)
            __set_term_info_x "1:256:0" nsterm xterm-256color
            ;;
        iterm2)
            _TM_ITERM2=1; export _TM_ITERM2
            __set_term_info_x "1:256:1:colon" iterm2 iTerm2.app xterm-256color
            ;;
        vte)
            __set_term_info_x "1:256:1:colon" vte-256color \
                gnome-256color vte gnome xterm-256color
            ;;
        wezterm)
            _TM_KITTY=1;  export _TM_KITTY
            _TM_ITERM2=1; export _TM_ITERM2
            __set_term_info_x "1:256:1:colon" wezterm wezterm-direct \
                xterm-256color vt525 vt525-basic
            ;;
        contour)
            __set_term_info_x "1:256:1:colon" contour contour-direct \
                xterm-256color vt525 vt525-basic
            ;;
        vt500c)
            __set_term_info_x "0:256:0" xterm-256color vt525 vt525-basic
            ;;
        vt500)
            __set_term_info_x "0:2:0" vt520 vt520-basic vt510
            ;;
        xterm420)
            __set_term_info_x "0:256:1:semi" xterm-256color
            ;;
        zutty)
            # Verified on Debian 12 zutty
            # Verified on Ubuntu 22.04 zutty
            # Colon prints junk (not just ignored)
            _TM_NOSTATUS=1; export _TM_NOSTATUS
            __set_term_info_x "0:256:1:semi" zutty xterm-256color
            ;;
        vt420)
            # There was NOT a color VT420 and capability code 22 didn't
            # exist until the 500 series, HOWEVER, I've yet to run
            # into a modern virtual terminal that pretends to support
            # vt200+ that doesn't support 256 colors.
            ###
            # This is in contrast to what I do with vt500, which is to
            # honor the capability code.  I do this because it was valid
            # for vt500 series, but not before.
            __set_term_info_x "0:256:0" vt420 vt420-basic xterm-256color
            ;;
        xterm340)
            __set_term_info_x "0:256:1:semi" xterm-vt340 xterm-256color
            ;;
        xterm320)
            __set_term_info_x "0:256:1:semi" xterm-vt320 xterm-256color
            ;;
        connectbot)
            # Surprisingly, no Emoji support
            _set_term_info_c "0:256:1:colon" xterm
            ;;
        vt320)
            # There was NOT a color VT340 and capability code 22 didn't
            # exist until the 500 series, HOWEVER, I've yet to run
            # into a modern virtual terminal that pretends to support
            # vt200+ that doesn't support 256 colors.
            _set_term_info_c "0:256:0" xterm
            ;;
        ghostty)
            __set_term_info_x "1:256:1:colon" ghostty xterm-ghostty \
                xterm-256color
            ;;
        vt240)
            # One of the few color supporting DEC terminals.
            _set_term_info_c "0:256:0" xterm-vt240 vt240 xterm
            ;;
        xterm240)
            __set_term_info_x "0:256:0" xterm-vt240 xterm-256color
            ;;
        xterm220)
            __set_term_info_x "0:2:0" xterm-vt220 vt220
            ;;
        kitty)
            # Verified on Ubuntu 22.04 kitty
            _TM_KITTY=1; export _TM_KITTY
            __set_term_info_x "1:256:1:colon" xterm-kitty kitty kitty-direct \
                xterm-256color
            ;;
        msterminal)
            __set_term_info_x "1:256:1:semi" \
                ms-terminal ms-vt100-color xterm-256color
            ;;
    esac
}

########################################
# Sets No Global
__cleanpath_pre ()
{
    if ! __e_setecho -b; then return 1; fi
    SCP_OUTPUT="$1"
    shift

    if [ -x "${BASE}/sbin/cleanpath" ]
    then
        CP="${BASE}/sbin/cleanpath -Pb ${SCP_OUTPUT} --"
    elif [ -x "${HOME}/sbin/cleanpath" ]
    then
        CP="${HOME}/sbin/cleanpath -Pb ${SCP_OUTPUT} --"
    else
        CP="sh_cleanpath ${SCP_OUTPUT} __PRE__"
    fi

    export $SCP_OUTPUT

    while [ ! -z "$1" ]
    do
        TWANT=`${CP} "$1"`
        shift
    done

    export $SCP_OUTPUT

    "$_ECHO" "$TWANT"
    unset TWANT
    unset P
    unset CP
    unset SCP_OUTPUT
}

########################################
# Sets No Global
__cleanpath_post ()
{
    if ! __e_setecho -b; then return 1; fi
    SCP_OUTPUT="$1"
    shift

    if [ -x "${BASE}/sbin/cleanpath" ]
    then
        CP="${BASE}/sbin/cleanpath -P ${SCP_OUTPUT} --"
    elif [ -x "${HOME}/sbin/cleanpath" ]
    then
        CP="${HOME}/sbin/cleanpath -P ${SCP_OUTPUT} --"
    else
        CP="sh_cleanpath ${SCP_OUTPUT} __POST__"
    fi

    while [ ! -z "$1" ]
    do
        TWANT=`${CP} "$1"`
        shift
    done

    "$_ECHO" "$TWANT"
    unset TWANT
    unset P
    unset CP
    unset SCP_OUTPUT
}

__my_inpath ()
{
    if ! __e_setsyscmd sed _SED; then return 1; fi

    FCX=0
    FVAR="PATH"
    if [ ! -z "$2" ]        # IF there are two arguments, the first subs PATH
    then
        FVAR="$1"
        shift               # Drop first argument
    fi
    FILE="$1"
    eval 'TWANT="${'`${_ECHO} ${FVAR}`'}"'

    for P in `"$_ECHO" $TWANT | $_SED -e's/:/ /g'`
    do
        if [ -e "${P}/${FILE}" ]
        then
            "$_ECHO" "${P}/${FILE}"
            FCX=`expr 1 + $FCX`
        fi
    done

    unset FVAR
    unset FILE
    unset TWANT

    if [ "0" -eq "${FCX}" ]
    then
        unset FCX
        return 1
    else
        unset FCX
        return 0
    fi
}

__my_inpath_quiet ()
{
    __my_inpath "$@" >/dev/null
}

__my_inpath_first ()
{
    __my_inpath "$@" | head -1
}

__q_terminfo_dirs ()
{
    if [ -z "${_TM_TERMINFO_DIRS}" ]
    then
        export TERMINFO_DIRS
        TERMINFO_DIRS=`__cleanpath_post TERMINFO_DIRS "/etc/terminfo"`
        TERMINFO_DIRS=`__cleanpath_post TERMINFO_DIRS "/usr/share/terminfo"`
        TERMINFO_DIRS=`__cleanpath_pre TERMINFO_DIRS "/lib/terminfo"`
        TERMINFO_DIRS=`__cleanpath_pre TERMINFO_DIRS "/usr/local/share/terminfo"`
        TERMINFO_DIRS=`__cleanpath_pre TERMINFO_DIRS "${HOME}/.terminfo"`
        export TERMINFO_DIRS
        _TM_TERMINFO_DIRS=1
        export _TM_TERMINFO_DIRS
    fi
}

__find_terminfo ()
{
    __q_terminfo_dirs
    ENTRY="$1"
    ST=`${_ECHO} ${ENTRY} | cut -c1`
    __my_inpath_quiet TERMINFO_DIRS "${ST}/${ENTRY}"
    if [ "0" = "$?" ]
    then
        unset ENTRY
        unset ST
        unset FOUND
        return 0
    fi
    SX=`${_ECHO} -n ${ST} | hexdump -n 1 -e '/1 "%02x"'`
    __my_inpath_quiet TERMINFO_DIRS "${SX}/${ENTRY}"
    if [ "0" = "$?" ]
    then
        unset ENTRY
        unset ST
        unset SX
        unset FOUND
        return 0
    fi

    unset ENTRY
    unset ST
    unset SX
    unset FOUND
    return 1
}

__set_cterm_fallback ()
{
    unset TCOLOR

    if [ ! -z "$_TM_COLORS" -a "0" -lt "$_TM_COLORS" ]; then
        TCOLOR="+c"
        if [ "8" -le "$_TM_COLORS" ]; then
            TCOLOR="-8color +color8 $TCOLOR"
        fi
        if [ "16" -le "$_TM_COLORS" ]; then
            TCOLOR="-16color +16color +color $TCOLOR"
        fi
        if [ "88" -le "$_TM_COLORS" ]; then
            TCOLOR="-88color +88color $TCOLOR"
        fi
        if [ "256" -le "$_TM_COLORS" ]; then
            TCOLOR="-256color +256color $TCOLOR"
        fi
    fi

    FALL="$1"
    while [ ! -z "$FALL" ]
    do
        for C in $TCOLOR
        do
            CFALL="${FALL}${C}"
            __find_terminfo "$CFALL"
            if [ "0" = "$?" ]; then
                TERM="$CFALL"; export TERM
                true; return
            fi
        done
        __find_terminfo "$FALL"
        if [ "0" = "$?" ]; then
            TERM="$FALL"; export TERM
            true; return
        fi

        shift
        FALL="$1"
    done

    false; return
}

__set_term_fallback ()
{
    FALL="$1"
    while [ ! -z "$FALL" ]
    do
        shift

        __find_terminfo "$FALL"
        if [ "0" = "$?" ]; then
            TERM="$FALL"; export TERM
            true; return
        fi

        FALL="$1"
    done

    false; return
}

__set_term_fallback_x ()
{
    __set_term_fallback $@
    if [ "0" != "$?" ]; then
        TERM="xterm"; export TERM
    fi
    true; return
}

__do_usage ()
{
    __e_setecho -b
    "$_ECHO" " "
    "$_ECHO" $0" [-q] [-h]"
}

__do_help ()
{
    __e_setecho -b
    "$_ECHO" $0
    "$_ECHO" " "
    "$_ECHO" "Tries to figure out which terminal emulator is being used."
    "$_ECHO" " "
    "$_ECHO" " -q | --quiet"
    "$_ECHO" "   Use if 'eval' of this is needed."
    "$_ECHO" " "
    "$_ECHO" " -h | --help"
    "$_ECHO" "   This help."
    "$_ECHO" " "
    "$_ECHO" 'To apply recommendation: "source '$0'" or "eval `'$0 -q'`"'
    "$_ECHO" " "
}

__q_getterm ()
{
    __e_setsyscmd grep GREP

    if [ ! -x "$_TERMREAD" ]; then
        if [ -x "${PWD}/termread" ]; then
            _TERMREAD="${PWD}/termread"
        elif [ -x "${HOME}/sbin/termread" ]; then
            _TERMREAD="${HOME}/sbin/termread"
        elif [ -x "${HOME}/bin/termread" ]; then
            _TERMREAD="${HOME}/bin/termread"
        else
            __e_setsyscmd -b termread _TERMREAD
        fi
    fi
    if [ ! -x "$_TERMREAD" ]; then
        if [ -r "${PWD}/termread.c" ]
        then
            __e_setsyscmd -b gcc _CC
            if [ ! -x "$_CC" ]; then
                __e_setsyscmd -b cc1 _CC
            fi
            if [ -x "$_CC" ]
            then
                $_CC -o termread termread.c
            fi
            if [ -x "${PWD}/termread" ]; then
                _TERMREAD="${PWD}/termread"
            fi
        fi
    fi
    if [ "termread" = "${_TERMREAD}" ]; then
        _TERMREAD="${PWD}/termread"
    fi
    if [ ! -x "$_TERMREAD" ]; then
        __debug_p "Unable to locate termread."
        return 1
    fi

    _TERMSET=0
    unset _TM_PUTTY
    unset _TM_KITTY
    unset _TM_ITERM2
    unset _TM_EMOJI

    eval `${_TERMREAD} '!' -t`
    __debug_p '...Primary DA "'$TERMID'".'

    if [ -z "${TERMID}" ]; then
        __debug_p "No response to 'Primary DA'"
        # If it didn't answer to the xterm user9, then no color.
        _TM_COLORS=0; export _TM_COLORS
        eval `TERM="vt52" ${_TERMREAD} -t`
        __debug_p "Read DECID '"${TERMID}"'."
        if [ -z "${TERMID}" ]
        then
            __debug_p "No response to 'DECID'"
            return 1
        elif [ '\033/Z' = "${TERMID}" ]
        then
            __debug_p "xterm in vt52 mode"
            # I can only assume this is an xterm running in vt52 emulation
            __set_term_fallback xterm-vt52 vt52-basic vt52 vt52h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/A' = "${TERMID}" ]
        then
            __debug_p "Response code for real vt50"
            __set_term_fallback vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/H' = "${TERMID}" ]
        then
            __debug_p "Response code for real vt50h"
            __set_term_fallback vt50h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/J' = "${TERMID}" ]
        then
            __debug_p "Response code for real vt50j"
            __set_term_fallback vt50j vt50h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/K' = "${TERMID}" ]
        then
            __debug_p "Response code for real vt52"
            __set_term_fallback vt52 vt50h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/L' = "${TERMID}" ]
        then
            __debug_p "Response code for real vt52b"
            __set_term_fallback vt52b vt52 vt50h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        fi
        return 1
    fi

    eval `"${_TERMREAD}" '!' -23e`
    if [ -n "$TERM2DA" ]
    then
        __debug_p '.Secondary DA "'$TERM2DA'".'
    else
        __debug_p '.Secondary DA is empty.'
    fi
    if [ -n "$TERM3DA" ]
    then
        __debug_p '..Tertiary DA "'$TERM3DA'".'
    else
        __debug_p '..Tertiary DA is empty.'
    fi
    if [ -n "$ENQ" ]
    then
        __debug_p '..........ENQ "'$ENQ'".'
    else
        __debug_p '..........ENQ is empty.'
    fi
    if [ "0" = "$_TERMSET" -a -n "$ENQ" ]
    then
        if [ "PuTTY" = "$ENQ" ]
        then
            __debug_p "PuTTY response"
            _known_terminal "putty"
        elif [ '\033[?1;2c' = "$ENQ" \
            -a '\033[?1;2c' = "$TERMID" \
            -a '\033[>85;95;0c' = "${TERM2DA}" ]
        then
            __debug_p "rxvt-unicode-256color (double enq)"
            _known_terminal "rxvt-unicode-256color"
        fi
    fi
    if [ "0" = "$_TERMSET" ]
    then
      case "$TERMID" in
        '\033[?6c')
            __debug_p "vt102 Primary DA response."
            if [ -z "$TERM2DA" ]
            then
                __debug_p "No Secondary DA, maybe linux console?"
                _known_terminal "vt102"
            else
              case "$TERM2DA" in
                '\033[>0;136;0c')
                    # PuTTY if ENQ is not set to PuTTY
                    __debug_p "vt102 + PuTTY response"
                    _known_terminal "putty"
                    ;;
                '\033[?6c')
                    __debug_p "JetBrains JediTerm (VT420 pretending to VT102)"
                    _known_terminal "JediTerm"
                    ;;
                '\033[>0;2'?'00;1c')
                    __debug_p "alacritty response"
                    _known_terminal "alacritty"
                    ;;
                '\033[>0;1901;1c')
                    __debug_p "alacritty response"
                    _known_terminal "alacritty"
                    ;;
                *)
                    __debug_p "Unknown vt102 clone."
                    _known_terminal "vt102"
                    ;;
              esac
            fi
            ;;
        '\033[?1;0c')
            # VT 101
            __debug_p "vt101 Primary DA response."
            if [ '\033[>0;10;1c' = "${TERM2DA}" ]
            then
                __debug_p "Windows Console / Microsoft Terminal (before 1.18.1421.0)"
                _known_terminal "msterm18"
            else
                __debug_p "VT101 Response, specific type unknown"
                _known_terminal "vt101"
            fi
            ;;
        '\033[?1;2c')
    # \e[?1;2c : Claims to be a vt100
            # vt100 could mean...
            __debug_p "vt100 Primary DA response."
            if [ '\033[>84;0;0c' = "$TERM2DA" ]
            then
                __debug_p "Byobu terminal"
                _known_terminal "byobu"
            elif [ '\033[>0;115;0c' = "$TERM2DA" ]
            then
                # Cool Retro Term or Konsole
                __debug_p "Konsole"
                _known_terminal "konsole"
            elif [ '\033[>1;95;0c' = "${TERM2DA}" ]
            then
                # NeXT or macOS Terminal.app
                __debug_p "Terminal.app"
                _known_terminal "terminal.app"
            elif [ '\033[>0;95;0c' = "${TERM2DA}" ]
            then
    # \e[?1;2c : Claims to be a vt100
                # iTerm2
                __debug_p "iTerm2.app (vt100 mode) up to ver 3.4.x"
                _known_terminal "iterm2"
            elif [ '\033[>85;95;0c' = "${TERM2DA}" ]
            then
                __debug_p "rxvt-unicode-256color"
                _known_terminal "rxvt-unicode-256color"
            fi
            if [ "0" = "$_TERMSET" ]
            then
    # \e[?1;2c : Claims to be a vt100
                if [ -z "$TERM2DA" ]
                then
                    # Just like a REAL vt100, no Secondary DA.
                    __debug_p "VT100 Primary DA response.  Set as VT100"
                else
                    # This HAS a secondary DA, but this doesn't have a
                    # match for it (yet)?
                    # Also, vt100 is a fairly basic terminal to claim to
                    # be, it's begging the user to not expect features.
                    __debug_p "Unknown vt100 Soft Terminal.  Set as VT100"
                fi
                _known_terminal "vt100"
            fi
            ;;
        '\033[?65;1;9c')
    # \e[?65;1;9c : VT500 - VTE on Debian/Ubuntu libvte-2.91
            # Would try to separate this more but
            # xfce4-terminal and gnome-terminal both
            # respond identically, and I've never found
            # a non-VTE terminal with this primary DA.
            __debug_p "vt500 - VTE response (Gnome, Xfce4)"
            # Should respond to TERM_BG and COLOR
            _known_terminal "vte"
            ;;
        '\033[?65;4;6;18;22c')
    # \e[?65;4;6;18;22c : VT500 - Wezterm with Sixel specific
            __debug_p "wezterm with sixel"
            _known_terminal "wezterm"
            ;;
        '\033[?65;22;314;1;28;4;8c')
            __debug_p "Contour"
            _known_terminal "contour"
            ;;
        '\033[?65;'*';22;'*c)
    # \e[?65;*c : VT500 Clone of Some Sort
            __debug_p "vt500 series or clone with color"
            _known_terminal "vt500c"
            ;;
        '\033[?65;'*c)
    # \e[?65;*c : VT500 Clone of Some Sort
            __debug_p "vt500 series or clone"
            _known_terminal "vt500"
            ;;
        '\033[?64;1;2;4;6;17;18;21;22c')
    # \e[?65;*c : VT420 -- iTerm2 v3.5 and up
            # from macOS M1 iTerm2 version 3.5.2 (homebrew)
            #       PRIMARY DA  : \e[?64;1;2;4;6;17;18;21;22c
            #       SECONDARY DA: \e[>41;2500;0c
            #       TERTIARY DA : \eP!I69547260\e\
            # from macOS M1 iTerm2 version 3.5??
            #       PRIMARY DA  : \e[?64;1;2;4;6;17;18;21;22c
            #       SECONDARY DA: \e[>0;95;0c
            #       TERTIARY DA : <null>
            __debug_p "iTerm2 from v 3.5"
            _known_terminal "iterm2"
            ;;
        '\033[?64;1;2;6;9;15;16;17;18;21;22;28c')
    # \e[?64;1;2;6;9;15;16;17;18;21;22;28c : VT420 mode of xterm
            __debug_p "xterm in vt420 mode"
            _known_terminal "xterm420"
            ;;
        '\033[?64;1;9;15;21;22c')
    # \e[?64;1;9;15;21;22c : VT420 mode of zutty
            __debug_p "zutty in vt420 mode"
            _known_terminal "zutty"
            ;;
        '\033[?64;'*c)
    # \e[?64;*c : VT4XX Clone of Some Sort
            __debug_p "vt420 series or clone"
            _known_terminal "vt420"
            ;;
        '\033[?63;1;2;6;9;15;16;22;28c')
    # \e[?63;1;2;6;9;15;16;22;28c : VT320 mode of xterm
            __debug_p "xterm in vt320 mode"
            _known_terminal "xterm320"
            ;;
        '\033[?63;1;2;4;6;9;15;16;22;28c')
    # \e[?63;1;2;4;6;9;15;16;22;28c : VT340 mode of xterm
            __debug_p "xterm in vt340 mode + sixel"
            _known_terminal "xterm340"
            ;;
        '\033[?63;'*c)
    # \e[?63;*c : VT3xx Clone of Some Sort
            if [ 'xterm-256color' = "$TERM2DA" ]
            then
                __debug_p "ConnectBot Android (vt340)"
                _known_terminal "connectbot"
            else
                case "$TERM2DA" in
                  '\033[>19;'*c)
                    __debug_p "vt340 or clone"
                    ;;
                  *)
                    __debug_p "vt320 or clone"
                    ;;
                esac
                _known_terminal "vt320"
            fi
            ;;
        '\033[?62;22c')
            # Ha, VT240 didn't have capability 22, though it DID have color
            __debug_p "vt240 (+ feature 22 [vt500 feature string])"
            case "$TERM2DA" in
                '\033[>1;10;0c')
                    # Ghostty (at least on macOS)
                    __debug_p "Ghostty"
                    _known_terminal "ghostty"
                    ;;
                *)
                    __debug_p "Secondary DA unrecognized"
                    _known_terminal "vt240"
                    ;;
            esac
            ;;
        '\033[?62;1;2;4;6;9;15;16;22;28c')
    # \e[?62;1;2;4;6;9;15;16;22;28c : VT240 mode of xterm
            __debug_p "xterm in vt240 mode (sixel)"
            _known_terminal "xterm240"
            ;;
        '\033[?62;1;2;6;9;15;16;22;28c')
    # \e[?62;1;2;6;9;15;16;22;28c : VT220 mode of xterm
            __debug_p "xterm in vt240 mode"
            _known_terminal "xterm240"
            ;;
        '\033[?62;1;2;4;6;8;9;15c')
            # Secondary ">1;123;0c"
            __debug_p "jvt220 (no terminfo)."
            _known_terminal "xterm240"
            ;;
        '\033[?62;1;4c')
    # \e[?62;1;4c : VT240 Clone...
            if [ '\033[>0;115;0c' = "$TERM2DA" ]
            then
                # Very old entry, not recently validated
                __debug_p "Konsole"
                _known_terminal "konsole-old"
            else
                __debug_p "Unknown vt240 descendent"
                _known_terminal "vt240"
            fi
            ;;
        '\033[?62'*';4'*c)
    # \e[?62;*4*c : VT240 Clone...
            __debug_p "VT240 Clone (sixel)"
            case "$TERM2DA" in
                '\033[>0;115;0c')
                    # Very old entry, not recently validated
                    __debug_p "Konsole"
                    _known_terminal "konsole-old"
                    ;;
                '\033[>0;95;0c')
                    # from macOS m4 iTerm2 version 3.4.23...
                    #       PRIMARY DA  : \e[?62;4c
                    #       SECONDARY DA: \e[>0;95;0c
                    #       TERTIARY DA : <null>
                    __debug_p "iTerm2 (vt240 mode) 3.4.23 or older"
                    _known_terminal "iterm2"
                    ;;
                '\033[>1;4000;'*c)
                    # ;15c (Ubuntu) and ;29c (macOS), so I'm starring it
                    __debug_p "Secondary DA looks like kitty"
                    _known_terminal "kitty"
                    ;;
                *)
                    __debug_p "Unknown vt220 descendent"
                    _known_terminal "vt240"
                    ;;
            esac
            ;;
        '\033[?62;'*c)
    # \e[?62;*c : VT200 Clone...
            __debug_p "vt200 or Clone"
            case "$TERM2DA" in
                '\033[>0;95;0c')
                    __debug_p "iTerm2 (vt220 mode) 3.4.23 or older"
                    _known_terminal "iterm2"
                    ;;
                '\033[>1;4000;'*c)
                    # ;15c (Ubuntu) and ;29c (macOS), so I'm starring it
                    __debug_p "Secondary DA looks like kitty"
                    _known_terminal "kitty"
                    ;;
                *)
                    __debug_p "Secondary DA unrecognized"
                    _known_terminal "vt240"
                    ;;
            esac
            ;;
        '\033[?61;1;21;22;28c')
            # \e[?61;1;21;22c : VTE on Manjaro libvte-2.91.0.8000
            # Would try to separate this more but
            # xfce4-terminal and gnome-terminal both
            # respond identically, and I've never found
            # a non-VTE terminal with this primary DA.
            __debug_p "Non-DEC-DEC - VTE response (Gnome, Xfce4)"
            _known_terminal vte
            ;;
        '\033[?61;1;21;22c')
            # \e[?61;1;21;22c : VTE on Manjaro libvte-2.91 (older)
            __debug_p "Non-DEC-DEC - VTE response (Gnome, Xfce4)"
            _known_terminal vte
            ;;
        '\033[?61;6;7;22;23;24;28;32;42c')
            __debug_p "Microsoft Terminal (since 1.18.1421.0)"
            _known_terminal "msterminal"
            ;;
        '\033[?61;6;7;21;22;23;24;28;32;42c')
            # Microsoft Terminal on Windows 11 v 1.19.10573.0
            # Microsoft Terminal on Windows 11 v 1.20.11381.0
            #       TERMID='\033[?61;6;7;21;22;23;24;28;32;42c'; export TERMID;
            #       TERM2DA='\033[>0;10;1c'; export TERM2DA;
            #       TERM3DA='\033P!|00000000\033\'; export TERM3DA;
            __debug_p "Microsoft Terminal (since 1.19.10573.0)"
            _known_terminal "msterminal"
            ;;
        '\033[?61;6;7;14;21;22;23;24;28;32;42c')
            # Microsoft Terminal on Windows 11 v 1.21.2911.0
            __debug_p "Microsoft Terminal (since 1.21.2911.0)"
            _known_terminal "msterminal"
            ;;
        '\033[?61;4;6;7;14;21;22;23;24;28;32;42c')
            # Microsoft Terminal on Windows 11 v 1.22.10731.0
            #TERM2DA='\033[>0;10;1c'; export TERM2DA; 
            #TERM3DA='\033P!|00000000\033\'; export TERM3DA; 
            __debug_p "Microsoft Terminal (since 1.22.10731.0 - sixel)"
            _known_terminal "msterminal"
            ;;
      esac
    fi

    if [ "1" = "$_TERMSET" ]
    then
        unset ENQ
        unset TERMID
        unset TERM2DA
        unset TERM3DA
        return 0
    fi

    __debug_p "TERMID='"$TERMID"'"
    __debug_p "TERM2DA='"$TERM2DA"'"
    __debug_p "Terminal Type Unknown: Trying to figure out color response."

    unset result
    eval `"${_TERMREAD}" '!' -c 231 --var result`
    if [ ! -z "$result" ]
    then
        _TM_COLORS=256
        export _TM_COLORS
        if [ "$TERM" = 'xterm' ]
        then
            __set_term_fallback_x xterm-256color xterm-16color xterm-color
        fi
        unset result
        return 0
    fi
    unset result
    eval `"${_TERMREAD}" '!' -c 14 --var result`
    if [ ! -z "$result" ]
    then
        _TM_COLORS=16
        export _TM_COLORS
        if [ "$TERM" = 'xterm' ]
        then
            __set_term_fallback_x xterm-16color xterm-color
        fi
        unset result
        return 0
    fi
    unset result
    eval `"${_TERMREAD}" '!' -c 6 --var result`
    if [ ! -z "$result" ]
    then
        _TM_COLORS=8
        export _TM_COLORS
        if [ "$TERM" = 'xterm' ]
        then
            __set_term_fallback_x xterm-color
        fi
        unset result
        return 0
    fi
    _TM_COLORS=0
    export _TM_COLORS
    return 1
}

DEBUG=1
ARGS=$*
for aa in $ARGS
do
    if [ "-q" = "$aa" ]
    then
        unset DEBUG
    elif [ "--quiet" = "$aa" ]
    then
        unset DEBUG
    elif [ "--help" = "$aa" ]
    then
        __do_help
        exit 0
    elif [ "-h" = "$aa" ]
    then
        __do_help
        exit 0
    else
        DEBUG=1 __debug_p "Unrecognized argument: "$aa
        __do_usage
        unset DEBUG
        exit 1
    fi
done
__q_getterm
if [ "0" = "$?" ]
then
    if [ -n "$DEBUG" ]
    then
        "$_ECHO" "# Recommended TERM:"
        "$_ECHO" "TERM=$TERM"
    fi
fi
unset DEBUG
unset _TERMREAD
unset ARGS
unset _SED
unset _CC
unset _ECHO
unset -f __debug_p
unset -f __e_setecho
unset -f __e_setsyscmd
unset -f __set_term_info_cx
unset -f __set_term_info_x
unset -f _known_terminal
unset -f __cleanpath_pre
unset -f __cleanpath_post
unset -f __my_inpath
unset -f __my_inpath_quiet
unset -f __my_inpath_first
unset -f __q_terminfo_dirs
unset -f __find_terminfo
unset -f __set_cterm_fallback
unset -f __set_term_fallback
unset -f __set_term_fallback_x
unset -f __do_usage
unset -f __do_help
unset -f __q_getterm
