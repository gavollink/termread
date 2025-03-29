#!/usr/bin/env sh
#############################################################################
export SAVETERM="${TERM}"

_debug_p ()
{
    _e_setecho -b
    if [ -n "$DEBUG" ]
    then
        "$_ECHO" $* >&2
    fi
}

########################################
# -b     Optional, attempt built-in echo OK if system echo cannot be found.
# Sets GLOBALS:
#  _ECHO
_e_setecho ()
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
#     _ECHO      (calls _e_setecho)
#     $ARG2
_e_setsyscmd ()
{
    if ! _e_setecho -b; then return 1; fi
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

########################################
# Sets No Global
_cleanpath_pre ()
{
    if ! _e_setecho -b; then return 1; fi
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
_cleanpath_post ()
{
    if ! _e_setecho -b; then return 1; fi
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

my_inpath ()
{
    if ! _e_setsyscmd sed _SED; then return 1; fi

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

my_inpath_quiet ()
{
    my_inpath "$@" >/dev/null
}

my_inpath_first ()
{
    my_inpath "$@" | head -1
}

_q_terminfo_dirs ()
{
    if [ -z "${_TM_TERMINFO_DIRS}" ]
    then
        export TERMINFO_DIRS
        TERMINFO_DIRS=`_cleanpath_post TERMINFO_DIRS "/etc/terminfo"`
        TERMINFO_DIRS=`_cleanpath_post TERMINFO_DIRS "/usr/share/terminfo"`
        TERMINFO_DIRS=`_cleanpath_pre TERMINFO_DIRS "/lib/terminfo"`
        TERMINFO_DIRS=`_cleanpath_pre TERMINFO_DIRS "/usr/local/share/terminfo"`
        TERMINFO_DIRS=`_cleanpath_pre TERMINFO_DIRS "${HOME}/.terminfo"`
        export TERMINFO_DIRS
        _TM_TERMINFO_DIRS=1
        export _TM_TERMINFO_DIRS
    fi
}

_find_terminfo ()
{
    _q_terminfo_dirs
    ENTRY="$1"
    ST=`${_ECHO} ${ENTRY} | cut -c1`
    my_inpath_quiet TERMINFO_DIRS "${ST}/${ENTRY}"
    if [ "0" = "$?" ]
    then
        unset ENTRY
        unset ST
        unset FOUND
        return 0
    fi
    SX=`${_ECHO} -n ${ST} | hexdump -n 1 -e '/1 "%02x"'`
    my_inpath_quiet TERMINFO_DIRS "${SX}/${ENTRY}"
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

_set_cterm_fallback ()
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
            _find_terminfo "$CFALL"
            if [ "0" = "$?" ]; then
                TERM="$CFALL"; export TERM
                true; return
            fi
        done
        _find_terminfo "$FALL"
        if [ "0" = "$?" ]; then
            TERM="$FALL"; export TERM
            true; return
        fi

        shift
        FALL="$1"
    done

    false; return
}

_set_term_fallback ()
{
    FALL="$1"
    while [ ! -z "$FALL" ]
    do
        shift

        _find_terminfo "$FALL"
        if [ "0" = "$?" ]; then
            TERM="$FALL"; export TERM
            true; return
        fi

        FALL="$1"
    done

    false; return
}

_set_term_fallback_x ()
{
    _set_term_fallback $@
    if [ "0" != "$?" ]; then
        TERM="xterm"; export TERM
    fi
    true; return
}

_do_usage ()
{
    _e_setecho -b
    "$_ECHO" " "
    "$_ECHO" $0" [-q] [-h]"
}

_do_help ()
{
    _e_setecho -b
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

_q_getterm ()
{
    _e_setsyscmd grep GREP

    if [ ! -x "$_TERMREAD" ]; then
        if [ -x "${PWD}/termread" ]; then
            _TERMREAD="${PWD}/termread"
        elif [ -x "${HOME}/sbin/termread" ]; then
            _TERMREAD="${HOME}/sbin/termread"
        elif [ -x "${HOME}/bin/termread" ]; then
            _TERMREAD="${HOME}/bin/termread"
        else
            _e_setsyscmd -b termread _TERMREAD
        fi
    fi
    if [ ! -x "$_TERMREAD" ]; then
        if [ -r "${PWD}/termread.c" ]
        then
            _e_setsyscmd -b gcc _CC
            if [ ! -x "$_CC" ]; then
                _e_setsyscmd -b cc1 _CC
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
        _debug_p "Unable to locate termread."
        return 1
    fi

    _TERMSET=0
    unset _TM_PUTTY
    unset _TM_KITTY
    unset _TM_ITERM2
    unset _TM_EMOJI

    eval `${_TERMREAD} '!' -t`
    _debug_p '...Primary DA "'$TERMID'".'

    if [ -z "${TERMID}" ]; then
        _debug_p "No response to 'Primary DA'"
        # If it didn't answer to the xterm user9, then no color.
        _TM_COLORS=0; export _TM_COLORS
        eval `TERM="vt52" ${_TERMREAD} -t`
        _debug_p "Read DECID '"${TERMID}"'."
        if [ -z "${TERMID}" ]
        then
            _debug_p "No response to 'DECID'"
            return 1
        elif [ '\033/Z' = "${TERMID}" ]
        then
            _debug_p "xterm in vt52 mode"
            # I can only assume this is an xterm running in vt52 emulation
            _set_term_fallback xterm-vt52 vt52-basic vt52 vt52h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/A' = "${TERMID}" ]
        then
            _debug_p "Response code for real vt50"
            _set_term_fallback vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/H' = "${TERMID}" ]
        then
            _debug_p "Response code for real vt50h"
            _set_term_fallback vt50h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/J' = "${TERMID}" ]
        then
            _debug_p "Response code for real vt50j"
            _set_term_fallback vt50j vt50h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/K' = "${TERMID}" ]
        then
            _debug_p "Response code for real vt52"
            _set_term_fallback vt52 vt50h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        elif [ '\033/L' = "${TERMID}" ]
        then
            _debug_p "Response code for real vt52b"
            _set_term_fallback vt52b vt52 vt50h vt50
            if [ "0" = "$?" ]; then
                _TERMSET=1
            fi
        fi
        return 1
    fi

    eval `"${_TERMREAD}" '!' -2 -3`
    if [ -n "$TERM2DA" ]
    then
        _debug_p '.Secondary DA "'$TERM2DA'".'
    else
        _debug_p '.Secondary DA is empty.'
    fi
    if [ -n "$TERM3DA" ]
    then
        _debug_p '..Tertiary DA "'$TERM3DA'".'
    else
        _debug_p '..Tertiary DA is empty.'
    fi
    case "$TERMID" in
        *PuTTY)
            _debug_p "PuTTY response"
            _TM_EMOJI=1;       export _TM_EMOJI
            _TM_PUTTY=1;       export _TM_PUTTY
            _TM_COLORS=256;    export _TM_COLORS
            # TRUECLR verified on Windows PuTTY 0.76 (colon ignored)
            _TM_TRUECLR=1;     export _TM_TRUCLR
            _TM_TRUEMODE=semi; export _TM_TRUEMODE
            _set_term_fallback_x "putty-256color" "putty" "xterm-256color"
            _TERMSET=1
            ;;
        '\033[?6c')
            _debug_p "vt102 Primary DA response."
            if [ -z "$TERM2DA" ]
            then
                ## THIS IS A BAD CALL, HONESTLY
                ## Nothing in the vt100 range originally responded to
                ## Secondary DA (it didn't exist yet), but
                ## every other terminal emulator gives us SOME
                ## other response.
                _debug_p "vt102 & no response on Secondary DA, maybe console"
                _TM_COLORS=8; export _TM_COLORS
                _set_term_fallback_x linux vt102 # st
                _TERMSET=1
            elif [ '\033[?6c' = "${TERM2DA}" ]
            then
                _debug_p "JetBrains JediTerm (VT420 pretending to VT102)"
                _TM_COLORS=256;     export _TM_COLORS
                _set_term_fallback_x jedi xterm-256color
                _TERMSET=1
            elif [ '\033[>0;2300;1c' = "${TERM2DA}" -a -z "${TERM3DA}" ]
            then
                _debug_p "vt102 + alacritty response"
                _TM_COLORS=256;     export _TM_COLORS
                _TM_TRUECLR=1;      export _TM_TRUCLR
                _TM_TRUEMODE=colon; export _TM_TRUEMODE
                _set_term_fallback_x alacritty rio
                _TERMSET=1
            elif [ '\033[>0;1901;1c' = "${TERM2DA}" -a -z "${TERM3DA}"  ]
            then
                _debug_p "vt102 + alacritty response"
                _TM_COLORS=256;     export _TM_COLORS
                _TM_TRUECLR=1;      export _TM_TRUCLR
                _TM_TRUEMODE=colon; export _TM_TRUEMODE
                _set_term_fallback_x alacritty rio
                _TERMSET=1
            elif [ '\033[>0;136;0c' = "${TERM2DA}" ]
            then
                # PuTTY if ENQ is not set to PuTTY
                _debug_p "vt102 + PuTTY response"
                _TM_EMOJI=1;       export _TM_EMOJI
                _TM_PUTTY=1;       export _TM_PUTTY
                _TM_COLORS=256;    export _TM_COLORS
                # TRUECLR verified on Windows PuTTY 0.76 (colon ignored)
                _TM_TRUECLR=1;     export _TM_TRUCLR
                _TM_TRUEMODE=semi; export _TM_TRUEMODE
                _set_term_fallback_x "putty-256color" "putty" "xterm-256color"
                _TERMSET=1
            else
                _debug_p "Unknown vt102 clone."
                # NOT PUTTY
                _TM_COLORS=0;   export _TM_COLORS
                _set_term_fallback_x vt102
                _TERMSET=1
            fi
            ;;
        '\033[?1;0c')
            # VT 101
            _debug_p "vt101 Primary DA response."
            if [ '\033[>0;10;1c' = "${TERM2DA}" ]
            then
                _debug_p "Windows Console / Microsoft Terminal (before 1.18.1421.0)"
                _TM_EMOJI=1;    export _TM_EMOJI
                _TM_COLORS=256; export _TM_COLORS
                _set_term_fallback_x ms-terminal ms-vt-utf8 \
                    ms-vt100+ ms-vt100-color xterm-256color
                _TERMSET=1
            else
                _debug_p "VT101 Response, specific type unknown"
                # VT101 or Descendent
                _TM_COLORS=0;   export _TM_COLORS
                _set_term_fallback_x vt101
                _TERMSET=1
            fi
            ;;
        '\033[?1;2c')
    # \e[?1;2c : Claims to be a vt100
            # vt100 could mean...
            _debug_p "vt100 Primary DA response."
            if [ '\033[>84;0;0c' = "$TERM2DA" ]
            then
                _debug_p "Byobu terminal"
                _TM_EMOJI=1;    export _TM_EMOJI
                _TM_COLORS=256; export _TM_COLORS
                _set_cterm_fallback xterm
                _TERMSET=1
            elif [ '\033[>0;115;0c' = "$TERM2DA" ]
            then
    # \e[?1;2c : Claims to be a vt100
                # Cool Retro Term or Konsole
                eval `"${_TERMREAD}" '!' -b`
                if [ -z "$TERM_BG" ]; then
                    ## Even though this flattens colors to pretend it's
                    ## a monochrome CRT, it still supports all the colors.
                    _debug_p "Cool-Retro-TERM"
                    _TM_EMOJI=1;      export _TM_EMOJI
                    _TM_COLORS=256;   export _TM_COLORS
                    _TM_EMOJISPACE=1; export _TM_EMOJISPACE
                    _set_term_fallback_x konsole
                else
                    _debug_p "Konsole"
                    _TM_EMOJI=1;       export _TM_EMOJI
                    _TM_COLORS=256;    export _TM_COLORS
                    _TM_TRUECLR=1;     export _TM_TRUCLR
                    # Internet claims new versions can do colon, but..
                    #  it doesn't work for me.
                    _TM_TRUEMODE=semi; export _TM_TRUEMODE
                    _set_term_fallback_x konsole-256color konsole xterm-256color
                fi
                _TERMSET=1
            elif [ '\033[>1;95;0c' = "${TERM2DA}"  -a "0" = "$_TERMSET" ]
            then
    # \e[?1;2c : Claims to be a vt100
                # NeXT or macOS Terminal.app
                _debug_p "Terminal.app"
                _TM_EMOJI=1;        export _TM_EMOJI
                _TM_COLORS=256;     export _TM_COLORS
                _set_cterm_fallback nsterm xterm-256color xterm
                _TERMSET=1
            elif [ '\033[>0;95;0c' = "${TERM2DA}"  -a "0" = "$_TERMSET" ]
            then
    # \e[?1;2c : Claims to be a vt100
                # iTerm2
                _debug_p "iTerm2.app (vt100 mode) up to ver 3.4.x"
                _TM_COLORS=256;     export _TM_COLORS
                _TM_ITERM2=1;       export _TM_ITERM2
                _TM_TRUECLR=1;      export _TM_TRUCLR
                _TM_TRUEMODE=colon; export _TM_TRUEMODE
                _set_term_fallback_x iterm2 iTerm2.app xterm-256color
                _TERMSET=1
            else
    # \e[?1;2c : Claims to be a vt100
                _TM_EMOJI=0;    export _TM_EMOJI
                _TM_COLORS=0;   export _TM_COLORS
                _debug_p "VT100 Primary DA response."
                _set_term_fallback_x vt100-basic
                _TERMSET=1
            fi
            ;;
        '\033[?1;2c\033[?1;2c')
            # ENQ responded with Primary DA (doubled)
            # vt100 could mean...
            _debug_p "vt100 Primary DA response (doubled from ENQ)."
            if [ '\033[>85;95;0c' = "${TERM2DA}" -a "0" = "$_TERMSET" ]
            then
                _TM_EMOJI=0;        export _TM_EMOJI
                _TM_COLORS=256;     export _TM_COLORS
                _TM_TRUECLR=1;      export _TM_TRUECLR
                _TM_TRUEMODE=colon; export _TM_TRUEMODE
                _debug_p "rxvt-unicode-256color"
                _set_term_fallback_x rxvt-unicode-256color xterm-256color
                _TERMSET=1
            else
                _TM_EMOJI=0;    export _TM_EMOJI
                _TM_COLORS=0;   export _TM_COLORS
                _debug_p "VT100 specifics not detected."
                _set_term_fallback_x vt100-basic
                _TERMSET=1
            fi
            ;;
        '\033[?65;1;9c')
    # \e[?65;1;9c : VT500 - VTE on Debian/Ubuntu libvte-2.91
            # Would try to separate this more but
            # xfce4-terminal and gnome-terminal both
            # respond identically, and I've never found
            # a non-VTE terminal with this primary DA.
            _debug_p "vt500 - VTE response (Gnome, Xfce4)"
            # Should respond to TERM_BG and COLOR
            _TM_EMOJI=1;        export _TM_EMOJI
            _TM_COLORS=256;     export _TM_COLORS
            _TM_TRUECLR=1;      export _TM_TRUCLR
            _TM_TRUEMODE=colon; export _TM_TRUEMODE
            _set_term_fallback_x vte-256color gnome-256color vte gnome \
                xterm-256color
            _TERMSET=1
            ;;
        '\033[?65;4;6;18;22c')
    # \e[?65;4;6;18;22c : VT500 - Wezterm with Sixel specific
            _debug_p "wezterm with sixel"
            _TM_EMOJI=1;        export _TM_EMOJI
            _TM_COLORS=256;     export _TM_COLORS
            _TM_TRUECLR=1;      export _TM_TRUCLR
            _TM_TRUEMODE=colon; export _TM_TRUEMODE
            _TM_KITTY=1;        export _TM_KITTY
            _TM_ITERM2=1;       export _TM_ITERM2
            _set_cterm_fallback wezterm-direct wezterm \
                vt525 vt525-basic vt520 vt520-basic \
                vt420-basic vt420 vt320-basic vt320 xterm
            _TERMSET=1
            ;;
        '\033[?65;'*c)
    # \e[?65;*c : VT500 Clone of Some Sort
            _debug_p "vt500 series or clone"
            _TM_COLORS=0; export _TM_COLORS
            _set_term_fallback_x vt525-basic vt525 vt520-basic vt520 \
                vt420-basic vt420 vt320-basic vt320 \
                vt300 vt220-basic vt220 vt200
            _TERMSET=1
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
            _debug_p "iTerm2 from v 3.5"
            _TM_COLORS=256;     export _TM_COLORS
            _TM_ITERM2=1;       export _TM_ITERM2
            _TM_TRUECLR=1;      export _TM_TRUCLR
            _TM_TRUEMODE=colon; export _TM_TRUEMODE
            _set_cterm_fallback iterm2 iTerm2.app xterm
            _TERMSET=1
            ;;
        '\033[?64;1;2;6;9;15;16;17;18;21;22;28c')
    # \e[?64;1;2;6;9;15;16;17;18;21;22;28c : VT420 mode of xterm
            _debug_p "xterm in vt420 mode"
            _TM_COLORS=256;    export _TM_COLORS
            _TM_TRUECLR=1;     export _TM_TRUCLR
            _TM_TRUEMODE=semi; export _TM_TRUEMODE
            _set_term_fallback_x xterm-256color
            _TERMSET=1
            ;;
        '\033[?64;1;9;15;21;22c')
    # \e[?64;1;9;15;21;22c : VT420 mode of zutty
            _debug_p "zutty in vt420 mode"
            _TM_EMOJI=0;    export _TM_EMOJI
            _TM_NOSTATUS=1; export _TM_EMOJI
            _TM_COLORS=256; export _TM_COLORS
            # Verified on Ubuntu 22.04 zutty
            _TM_TRUECLR=1; export _TM_TRUCLR
            # Colon prints junk (not just ignored)
            _TM_TRUEMODE=semi; export _TM_TRUEMODE
            _set_term_fallback_x zutty vt420-basic vt420 xterm-256color
            _TERMSET=1
            ;;
        '\033[?64;'*c)
    # \e[?64;*c : VT4XX Clone of Some Sort
            _debug_p "vt420 series or clone"
            _TM_EMOJI=0;  export _TM_EMOJI
            _TM_COLORS=0; export _TM_COLORS
            _set_term_fallback_x vt420-basic vt420 vt320-basic vt320 \
                vt300 vt220-basic vt220 vt200
            _TERMSET=1
            ;;
        '\033[?63;1;2;6;9;15;16;22;28c')
    # \e[?63;1;2;6;9;15;16;22;28c : VT320 mode of xterm
            _debug_p "xterm in vt320 mode"
            _TM_COLORS=256; export _TM_COLORS
            _set_term_fallback_x xterm-vt320 vt320 vt300 xterm-256color
            _TERMSET=1
            ;;
        '\033[?63;1;2;4;6;9;15;16;22;28c')
    # \e[?63;1;2;4;6;9;15;16;22;28c : VT340 mode of xterm
            _debug_p "xterm in vt340 mode"
            _TM_COLORS=256; export _TM_COLORS
            _set_term_fallback_x xterm-vt340 vt340 xterm-vt320 \
                vt320 vt300 xterm-256color
            _TERMSET=1
            ;;
        '\033[?63;'*c)
    # \e[?63;*c : VT3xx Clone of Some Sort
            _debug_p "vt320 or Clone"
            if [ 'xterm-256color' = "$TERM2DA" ]
            then
                _debug_p "ConnectBot Android"
                _TM_COLORS=256; export _TM_COLORS
                _set_cterm_fallback xterm
                _TERMSET=1
            else
                _debug_p "Unknown subtype"
                _TM_COLORS=0; export _TM_COLORS
                _set_term_fallback_x vt320-basic vt320 vt300
                _TERMSET=1
            fi
            ;;
        '\033[?62;22c\033[>'*c)
            _debug_p "Ghostty doubled PrimaryDA response"
            case "$TERM2DA" in
                '\033[>1;10;0c')
                    # Ghostty (at least on macOS)
                    _debug_p "Ghostty"
                    _TM_COLORS=256; export _TM_COLORS
                    _TM_TRUECLR=1;  export _TM_TRUCLR
                    _set_term_fallback_x Ghostty ghostty xterm-ghostty \
                        vt240 xterm-vt220 vt220 vt200 xterm-256color
                    _TERMSET=1
                    ;;
                default)
                    _debug_p "Secondary DA unrecognized"
                    _TM_COLORS=0; export _TM_COLORS
                    _set_term_fallback_x xterm-vt240 vt240 xterm-vt220 \
                        vt220 vt200 xterm-256color
                    _TERMSET=1
                    ;;
            esac
            ;;
        '\033[?62;22c')
            _debug_p "VT240 Clone ([?62; with feature 22)"
            case "$TERM2DA" in
                '\033[>1;10;0c')
                    # Ghostty (at least on macOS)
                    _debug_p "Ghostty"
                    _TM_COLORS=256; export _TM_COLORS
                    _TM_TRUECLR=1;  export _TM_TRUCLR
                    _set_term_fallback_x Ghostty ghostty xterm-ghostty \
                        vt240 xterm-vt220 vt220 vt200 xterm-256color
                    _TERMSET=1
                    ;;
                default)
                    _debug_p "Secondary DA unrecognized"
                    _TM_COLORS=0; export _TM_COLORS
                    _set_term_fallback_x xterm-vt240 vt240 xterm-vt220 \
                        vt220 vt200 xterm-256color
                    _TERMSET=1
                    ;;
            esac
            ;;
        '\033[?62;1;2;4;6;9;15;16;22;28c')
    # \e[?62;1;2;4;6;9;15;16;22;28c : VT240 mode of xterm
            _debug_p "xterm in vt240 mode ([?62; with feature 4)"
            _TM_COLORS=256; export _TM_COLORS
            _set_term_fallback_x xterm-vt240 vt240 xterm-vt220 \
                vt220 vt200 xterm-256color
            _TERMSET=1
            ;;
        '\033[?62;1;2;6;9;15;16;22;28c')
    # \e[?62;1;2;6;9;15;16;22;28c : VT220 mode of xterm
            _debug_p "xterm in vt220 mode ([?62; without feature 4)"
            _TM_COLORS=256; export _TM_COLORS
            _set_term_fallback_x xterm-vt220 vt220 vt200 xterm-256color
            _TERMSET=1
            ;;
        '\033[?62;1;2;4;6;8;9;15c')
            # Secondary ">1;123;0c"
            _debug_p "jvt220 (no terminfo)."
            _TTY_COLORS=256; export _TTY_COLORS
            _set_term_fallback_x xterm-vt220 vt220 vt200 xterm-256color
            _TERMSET=1
            ;;
        '\033[?62'*';4'*'c')
    # \e[?62;*4*c : VT240 Clone...
            _debug_p "VT240 Clone (feature 4)"
            case "$TERM2DA" in
                '\033[>0;115;0c')
                    # Very old entry, not recently validated
                    _debug_p "Konsole"
                    _TM_EMOJI=1;    export _TM_EMOJI
                    _TM_COLORS=256; export _TM_COLORS
                    _set_term_fallback_x konsole-256color konsole xterm-256color
                    _TERMSET=1
                    ;;
                '\033[>0;95;0c')
                    # from macOS m4 iTerm2 version 3.4.23...
                    #       PRIMARY DA  : \e[?62;4c
                    #       SECONDARY DA: \e[>0;95;0c
                    #       TERTIARY DA : <null>
                    _debug_p "iTerm2 (vt240 mode) 3.4.23 or older"
                    _TM_COLORS=256; export _TM_COLORS
                    _TM_EMOJI=1;    export _TM_EMOJI
                    _set_cterm_fallback iterm2 iTerm2.app xterm
                    _TERMSET=1
                    ;;
                '\033[>1;4000;'*c)
                    # ;15c (Ubuntu) and ;29c (macOS), so I'm starring it
                    _debug_p "Secondary DA looks like kitty"
                    _TM_COLORS=256;     export _TM_COLORS
                    _TM_EMOJI=1;        export _TM_EMOJI
                    _TM_KITTY=1;        export _TM_KITTY
                    # Verified on Ubuntu 22.04 kitty
                    _TM_TRUECLR=1;      export _TM_TRUCLR
                    _TM_TRUEMODE=colon; export _TM_TRUEMODE
                    _set_cterm_fallback xterm-kitty kitty-direct kitty \
                        xterm-vt240 vt240 xterm-vt220 vt220 vt200 \
                        xterm
                    _TERMSET=1
                    ;;
                default)
                    _debug_p "Unknown vt240 descendent"
                    _TM_COLORS=0;   export _TM_COLORS
                    _set_term_fallback_x vt220-basic
                    _TERMSET=1
                    ;;
            esac
            ;;
        '\033[?62;1;4c')
    # \e[?62;1;4c : VT240 Clone...
            if [ '\033[>0;115;0c' = "$TERM2DA" ]
            then
                # Very old entry, not recently validated
                _debug_p "Konsole"
                _TM_EMOJI=1;    export _TM_EMOJI
                _TM_COLORS=256; export _TM_COLORS
                _set_term_fallback_x konsole-256color konsole xterm-256color
                _TERMSET=1
            else
                _debug_p "Unknown vt240 descendent"
                _TM_COLORS=0;   export _TM_COLORS
                _set_term_fallback_x vt220-basic
                _TERMSET=1
            fi
            ;;
        '\033[?62;'*c)
    # \e[?62;*c : VT200 Clone...
            _debug_p "vt200 or Clone"
            case "$TERM2DA" in
                '\033[>0;95;0c')
                    _debug_p "iTerm2 (vt220 mode) 3.4.23 or older"
                    _TM_COLORS=256; export _TM_COLORS
                    _TM_EMOJI=1;    export _TM_EMOJI
                    _set_cterm_fallback iterm2 iTerm2.app xterm-vt220 xterm
                    _TERMSET=1
                    ;;
                '\033[>1;4000;'*c)
                    # ;15c (Ubuntu) and ;29c (macOS), so I'm starring it
                    _debug_p "Secondary DA looks like kitty"
                    _TM_COLORS=256;     export _TM_COLORS
                    _TM_EMOJI=1;        export _TM_EMOJI
                    _TM_KITTY=1;        export _TM_KITTY
                    # Verified on Ubuntu 22.04 kitty
                    _TM_TRUECLR=1;      export _TM_TRUCLR
                    _TM_TRUEMODE=colon; export _TM_TRUEMODE
                    _set_cterm_fallback xterm-kitty kitty-direct kitty \
                        xterm-vt240 vt240 xterm-vt220 vt220 vt200 \
                        xterm
                    _TERMSET=1
                    ;;
                default)
                    _debug_p "Secondary DA unrecognized"
                    _TM_COLORS=0; export _TM_COLORS
                    _set_term_fallback_x xterm-vt240 vt240 xterm-vt220 \
                        vt220 vt200 xterm-256color
                    _TERMSET=1
                    ;;
            esac
            ;;
        '\033[?61;1;21;22c')
            # \e[?61;1;21;22c : VTE on Manjaro libvte-2.91
            # Would try to separate this more but
            # xfce4-terminal and gnome-terminal both
            # respond identically, and I've never found
            # a non-VTE terminal with this primary DA.
            _debug_p "Non-DEC-DEC - VTE response (Gnome, Xfce4)"
            # Should respond to TERM_BG and COLOR
            _TM_EMOJI=1;        export _TM_EMOJI
            _TM_COLORS=256;     export _TM_COLORS
            _TM_TRUECLR=1;      export _TM_TRUCLR
            _TM_TRUEMODE=colon; export _TM_TRUEMODE
            _set_term_fallback_x vte-256color gnome-256color vte gnome \
                xterm-256color
            _TERMSET=1
            ;;
        '\033[?61;6;7;22;23;24;28;32;42c')
            _debug_p "Microsoft Terminal (since 1.18.1421.0)"
            _TM_COLORS=256;    export _TM_COLORS
            _TM_EMOJI=1;       export _TM_EMOJI
            _TM_TRUECLR=1;     export _TM_TRUCLR
            _TM_TRUEMODE=semi; export _TM_TRUEMODE
            _set_cterm_fallback ms-terminal ms-vt-utf8 \
                ms-vt100+ ms-vt100-color xterm-256color \
                xterm
            _TERMSET=1
            ;;
        '\033[?61;6;7;21;22;23;24;28;32;42c')
            # Microsoft Terminal on Windows 11 v 1.19.10573.0
            # Microsoft Terminal on Windows 11 v 1.20.11381.0
            #       TERMID='\033[?61;6;7;21;22;23;24;28;32;42c'; export TERMID;
            #       TERM2DA='\033[>0;10;1c'; export TERM2DA;
            #       TERM3DA='\033P!|00000000\033\'; export TERM3DA;
            _debug_p "Microsoft Terminal (since 1.19.10573.0)"
            _TM_COLORS=256;     export _TM_COLORS
            _TM_EMOJI=1;        export _TM_EMOJI
            # Verified on Windows 11
            _TM_TRUECLR=1;      export _TM_TRUCLR
            _TM_TRUEMODE=colon; export _TM_TRUEMODE
            _set_cterm_fallback ms-terminal ms-vt-utf8 \
                ms-vt100+ ms-vt100-color xterm-256color \
                xterm
            _TERMSET=1
            ;;
        '\033[?61;6;7;14;21;22;23;24;28;32;42c')
            # Microsoft Terminal on Windows 11 v 1.21.2911.0
            _debug_p "Microsoft Terminal (since 1.21.2911.0)"
            _TM_COLORS=256;     export _TM_COLORS
            _TM_EMOJI=1;        export _TM_EMOJI
            # Verified on Windows 11
            _TM_TRUECLR=1;      export _TM_TRUCLR
            _TM_TRUEMODE=colon; export _TM_TRUEMODE
            _set_cterm_fallback ms-terminal ms-vt-utf8 \
                ms-vt100+ ms-vt100-color xterm-256color \
                xterm
            _TERMSET=1
            ;;
        '\033[?61;4;6;7;14;21;22;23;24;28;32;42c')
            # Microsoft Terminal on Windows 11 v 1.22.10731.0
            #TERM2DA='\033[>0;10;1c'; export TERM2DA; 
            #TERM3DA='\033P!|00000000\033\'; export TERM3DA; 
            _debug_p "Microsoft Terminal (since 1.22.10731.0)"
            _TM_COLORS=256;     export _TM_COLORS
            _TM_EMOJI=1;        export _TM_EMOJI
            # Verified on Windows 11
            _TM_TRUECLR=1;      export _TM_TRUCLR
            _TM_TRUEMODE=colon; export _TM_TRUEMODE
            _set_cterm_fallback ms-terminal ms-vt-utf8 \
                ms-vt100+ ms-vt100-color xterm-256color \
                xterm
            _TERMSET=1
            ;;
    esac

    if [ "1" = "$_TERMSET" ]
    then
        unset TERMID
        unset TERM2DA
        unset TERM3DA
        return 0
    fi

    _debug_p "TERMID='"$TERMID"'"
    _debug_p "TERM2DA='"$TERM2DA"'"
    _debug_p "Terminal Type Unknown: Trying to figure out color response."

    unset result
    eval `"${_TERMREAD}" '!' -c 231 --var result`
    if [ ! -z "$result" ]
    then
        _TM_COLORS=256
        export _TM_COLORS
        if [ "$TERM" = 'xterm' ]
        then
            _set_term_fallback_x xterm-256color xterm-16color xterm-color
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
            _set_term_fallback_x xterm-16color xterm-color
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
            _set_term_fallback_x xterm-color
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
        _do_help
        exit 0
    elif [ "-h" = "$aa" ]
    then
        _do_help
        exit 0
    else
        DEBUG=1 _debug_p "Unrecognized argument: "$aa
        _do_usage
        unset DEBUG
        exit 1
    fi
done
_q_getterm
if [ "0" = "$?" ]
then
    if [ -n "$DEBUG" ]
    then
        "$_ECHO" "# Recommended TERM:"
    fi
    "$_ECHO" "TERM=$TERM"
fi
unset DEBUG
unset _TERMREAD
unset ARGS
unset _SED
unset _CC
unset _ECHO
