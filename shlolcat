#!/bin/sh

# shlolcat - lolcat written as a posix shell script and friends.
# Copyright 2019 Avi Halachmi (:avih) avihpit@yahoo.com
# License: MIT
# Home page: https://github.com/avih/shlolcat

esc=yes colors=true off= min=80 max=255 dx=10 dy=30 ab=
tab=8  # assumption about the terminal's tab width

usage() {
    echo "Usage: shlolcat [OPTIONS]... [FILE]..."
    echo "Concatenate files and print to standard output in rainbow colors."
    echo "With no FILE, or when FILE is -, read standard input."
    echo
    echo "  -h             Print this help and exit."
    echo "  -e             Strip escape sequences from input."
    echo "  -c COLORS      (1|8|256|true) Colors to use. Default: -c $colors."
    echo "  -a MIN -b MAX  (0-255) R/G/B values limits. Default: -a $min -b $max."
    echo "  -x DX  -y DY   (0-255) char/line color advancement. Default: -x $dx -y $dy."
    echo "  -o OFFSET      (0-100) Offset the pattern. Default: random."
    echo
    echo "Requires: od, cksum, date."
    echo "Home page: https://github.com/avih/shlolcat"
}

err_exit() {
    [ "${1-}" ] && >&2 printf "%s: %s\n" "${0##*/}" "$1"
    [ "${2-}" ] && >&2 printf "Usage: %s [-$opts] [FILE]...\n" "${0##*/}"
    exit ${3:-1}
}

intopt() {  # $1:opt-letter, $2:value, $3:target-var, $4:min, $5:max
    case x${2#-} in x*[!0123456789]*|x) false; esac \
    && [ "${#2}" -le 9 ] && [ "$2" -ge "$4" ] && [ "$2" -le "$5" ] \
    && eval $3=\$2 || err_exit "illegal value: -$1 $2  ($4..$5)"
}

opts=a:b:c:eho:x:y:
while getopts $opts o; do
    case $o in
        h) usage; exit 0;;
        e) esc=no;;
        c) case $OPTARG in 1|8|2|256|t|true) colors=$OPTARG;; *) err_exit "bad COLORS"; esac;;
        o) intopt "$o" "$OPTARG" off 0 100;;
        a) intopt "$o" "$OPTARG" min 0 255; ab=1;;
        b) intopt "$o" "$OPTARG" max 0 255; ab=1;;
        x) intopt "$o" "$OPTARG" dx 0 255;;
        y) intopt "$o" "$OPTARG" dy 0 255;;
        *) err_exit "" x;;
    esac
done
shift $((OPTIND-1))

[ $min -le $max ] || err_exit "MIN is bigger than MAX"
case $colors in 1|8) [ "$ab" ] || min=0 max=255; esac  # set full range if unset

# list_as_octals prints input bytes as 3-digits octal values, several per line
list_as_octals() { od -b -v -A n "$@"; }
rand() { sd=$(echo ${sd:-$(date)$$}|cksum); rnd=$((1${sd%? *}%$1)); } # [0..$1)

case $colors in
    # define add_cc() for the -c value: adds color+octal to $line as printf fmt
    # the 256 colors approximation is highly inaccurate, but reasonably useful.
    t|true) add_cc() { buf=$buf"\033[38;2;$r;$g;${b}m\\$1"; };;
     2|256) add_cc() { buf=$buf"\033[38;5;$((16 + 36*(r/45) + 6*(g/45) + b/45))m\\$1"; };;
         8) add_cc() { buf=$buf"\033[$((30 + 4*(b>127) + 2*(g>127) + (r>127) ))m\\$1"; };;
         1) add_cc() { buf=$buf"\033[$(( (r+g+b) > 383 ))m\\$1"; };; # no 22 on vt102
esac

# advance color $r $g $b by $1 (0-255). state is maintained at $s .
color_cycle() {
    case $s in
        G+) g=$((g+$1)); [ $max -gt $g ] || g=$max s=R- ;;
        R-) r=$((r-$1)); [ $min -lt $r ] || r=$min s=B+ ;;
        B+) b=$((b+$1)); [ $max -gt $b ] || b=$max s=G- ;;
        G-) g=$((g-$1)); [ $min -lt $g ] || g=$min s=R+ ;;
        R+) r=$((r+$1)); [ $max -gt $r ] || r=$max s=B- ;;
        B-) b=$((b-$1)); [ $min -lt $b ] || b=$min s=G+ ;;
    esac
}

# main, pipefail-like: after od succeeds/fails, we print its exit code in a new
# line and without trailing newline. The last read processes this lone value
# but returns error (EOF - no final \n), so we can identify the value easily.

{ list_as_octals -- "$@" && printf \\n0 || printf \\n$?; } | {
    r=$max g=$min b=$min s=G+
    [ "$off" ] || { rand 100; off=$rnd; }  # 100x16 is full cycle with -a0 -b255
    while [ $off -gt 0 ]; do color_cycle 16; off=$((off-1)); done

    R=$r G=$g B=$b S=$s  # saved for next line
    x=0 e=0 buf= fl= # column for tab tracking, esc state, output buffer, flush

    for sig in 2 15; do trap 'printf \\033[0m; exit '$((128+sig)) $sig; done

    while read octals; do
      for o in $octals; do
        case $e$o in
        0033)  # esc
            [ "$esc" = no ] || buf=$buf\\$o
            e=1
            ;;
        0012)  # newline
            buf=$buf\\n x=0
            r=$R g=$G b=$B s=$S
            color_cycle $dy
            R=$r G=$g B=$b S=$s
            ;;
        0011)  # tab
            color_cycle $dx
            add_cc $o
            x=$((x+1))
            while [ $((x % tab)) != 0 ]; do
                color_cycle $dx
                x=$((x+1))
            done
            ;;
        0[013]??)  # ascii7 or utf8 leading byte
            color_cycle $dx
            add_cc $o
            x=$((x+1))
            ;;
        0???)  # utf8 continuation byte
            buf=$buf\\$o
            ;;
        1???)  # first char after esc. 133 is '[', 050/051 are '(',')' stay(rom)
            [ $esc = no ] || buf=$buf\\$o
            case $o in 133) e=2;; 05[01]) ;; *) e=0; esac
            ;;
        2???)  # inside CSI sequence
            [ $esc = no ] || buf=$buf\\$o
            case $o in 1??) e=0; esac
            ;;
        esac
      done
      # flush $buf every other read (two od lines, up to ~800 bytes at $buf)
      [ "$fl" ] && { printf "$buf"; buf= fl=; } || fl=x
    done

    printf "$buf\033[0m"
    e=$octals  # last read is od exit code. print usage if e<128 (not signal)
    [ $e -gt 127 ] && exit $e || [ $e = 0 ] || err_exit "" x $e
}
