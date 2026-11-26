#!/usr/bin/env bash
# Static styling mockup only: no filesystem access, editor changes, or key input.
# Colors are based on data/oblivion.json. No icon font is required.
set -euo pipefail
# Character-based clipping/padding for the single-column glyphs used below.
export LC_ALL=C.UTF-8

color=auto
focus=sidebar
follow=on
for arg in "$@"; do
    case "$arg" in
        --plain) color=off ;;
        --color) color=on ;;
        --editor-focus) focus=editor ;;
        --no-follow) follow=off ;;
        --help|-h)
            printf '%s\n' \
                'Usage: sidebar-tree.sh [--plain|--color] [--editor-focus] [--no-follow]' \
                'Static preview, 92 columns wide. It does not modify or open files.' \
                '  › marks tree selection; ● marks the file open in the editor.' \
                '  --editor-focus previews the inactive tree selection.' \
                '  --no-follow previews selecting a different file without opening it.'
            exit 0 ;;
        *) printf 'Unknown option: %s\n' "$arg" >&2; exit 2 ;;
    esac
done

reset='' text='' muted='' directory='' active='' selected='' heading=''
if [[ "$color" == on ]] ||
   [[ "$color" == auto && -t 1 && -z "${NO_COLOR+x}" && "${TERM:-dumb}" != dumb ]]; then
    reset=$'\e[0m'
    text=$'\e[38;2;211;215;207m\e[48;2;46;52;54m'
    muted=$'\e[38;2;136;138;133m\e[48;2;46;52;54m'
    directory=$'\e[38;2;114;159;207m\e[48;2;46;52;54m'
    active=$'\e[38;2;238;238;236m\e[48;2;46;52;54m'
    selected=$'\e[38;2;238;238;236m\e[48;2;85;87;83m'
    heading=$'\e[1m\e[38;2;114;159;207m\e[48;2;46;52;54m'
    if [[ "$focus" == editor ]]; then
        selected=$'\e[38;2;161;164;158m\e[48;2;52;57;58m'
    fi
fi

left_width=24
right_width=67
padded() {
    local value=$1 width=$2
    # Bash printf field widths count bytes, so pad separately after clipping.
    # All mock labels use ASCII or single-column symbols, not arbitrary filenames.
    value=${value:0:width}
    printf '%s%*s' "$value" "$((width - ${#value}))" ''
}

rule() {
    local spaces
    printf -v spaces '%*s' "$1" ''
    printf '%s' "${spaces// /─}"
}

row() {
    local left=$1 right=$2 style=${3:-$text} divider=${4:-│}
    printf '%s' "$style"
    padded "$left" "$left_width"
    printf '%s%s%s' "$muted" "$divider" "$text"
    padded "$right" "$right_width"
    printf '%s\n' "$reset"
}

left_rule=$(rule "$left_width")
right_rule=$(rule "$right_width")
row " Project  [follow: $follow]" ' src/views/sidebar.cpp' "$heading"
row "$left_rule" "$right_rule" "$muted" '┼'

tree=(
    '▾ medit/'
    '  ▸ data/'
    '  ▸ docs/'
    '  ▸ lib/'
    '  ▾ src/'
    '    ▸ core/'
    '    ▸ files/'
    '    ▸ modes/'
    '    ▾ views/'
    '      editor.cpp'
    '      mainwindow.cpp'
    '      sidebar.cpp'
    '      sidebar.h'
    '  ▸ test/'
    '  CMakeLists.txt'
    '  README.md'
    ''
    ''
)
code=(
    '  1  #include "sidebar.h"'
    '  2  #include "screen/iscreen.h"'
    '  3'
    '  4  void SideBar::draw(IScreen &screen) {'
    '  5      if (!visible()) {'
    '  6          return;'
    '  7      }'
    '  8'
    '  9      // A dedicated tree view will render here.'
    ' 10      // The sidebar only manages the window region.'
    ' 11'
    ' 12  }'
)

selection=11
[[ "$follow" == off ]] && selection=12
for ((i = 0; i < ${#tree[@]}; ++i)); do
    label=${tree[i]}
    style=$text
    marker=' '
    current=' '
    [[ "$label" == *'/' ]] && style=$directory
    if ((i == 11)); then current='●'; style=$active; fi
    if ((i == selection)); then marker='›'; style=$selected; fi
    if [[ -n "$label" ]]; then label="$marker$current $label"; fi
    row "$label" "${code[i]-}" "$style"
done

printf '%s%s┴%s%s\n' "$muted" "$left_rule" "$right_rule" "$reset"
printf '%s' "$text"
padded " $focus · F3 panel · F6 focus · Enter open · h/l collapse/expand · follow: $follow" 92
printf '%s\n' "$reset"
printf '\n%s\n' 'Styling mockup only. › selected row; ● file open in editor. Follow control is a proposal.'
