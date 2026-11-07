#!/usr/bin/env bash
# tools/setup.sh: get everything else the build needs, and check it. Run it from the repository root (it is safe to
# run again: what is already in place and verified is kept):
#
#   tools/setup.sh
#
# 1. The submodules (splat and asm-processor at their pinned commits), if the clone was made without --recursive.
# 2. A Python virtual environment in .venv with the packages in requirements.txt, installed by pip with
#    --require-hashes (each package pinned to the version the byte-identical build was made with; pip builds three
#    of them from source and fetches setuptools for that, which the hashes do not cover).
# 3. IDO 5.3, the game's original C compiler (Silicon Graphics), as a static recompilation for x86-64 Linux, downloaded
#    into tools/ido_5.3/usr/lib. It is proprietary and not part of this repository.
# 4. The Nintendo 64 SDK headers (libultra 2.0I) that the source includes: the 19 files the build reads are in
#    include/2.0I, unmodified. If they are missing or changed,  tools/setup.sh --sdk-headers DIR  copies them from DIR,
#    the 2.0I include folder of an SDK copy you have (the folder that holds ultra64.h and PR/).
#
# 3 and 4 are checked file by file against the SHA-256 lists in tools/checksums/ before the build can use them: a file
# that does not match stops the setup. Nothing from your ROM is used here.
set -euo pipefail
cd "$(dirname "$0")/.."

IDO_URL="https://github.com/decompals/ido-static-recomp/releases/download/per-function/ido-5.3-recomp-ubuntu-latest.tar.gz"
IDO_TAR_SHA256="ed4d4c1c08a7b53528d19a669caef2b5fe961720ba99eb1b4074de9d59fe7826"
IDO_DIR="tools/ido_5.3/usr/lib"

say()  { printf '%s\n' "$*"; }
fail() { printf 'setup: %s\n' "$*" >&2; exit 1; }

SDK_SRC=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --sdk-headers)   [[ $# -ge 2 ]] || fail "--sdk-headers needs a folder"; SDK_SRC="$2"; shift 2 ;;
        --sdk-headers=*) SDK_SRC="${1#*=}"; shift ;;
        -h|--help)       sed -n '2,18p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *)               fail "unknown option: $1 (see tools/setup.sh --help)" ;;
    esac
done

# --- 0. the programs the setup and the build call ------------------------------------------------------------------
missing=()
for cmd in git python3 curl sha256sum tar ninja iconv mips-linux-gnu-as mips-linux-gnu-ld mips-linux-gnu-objcopy; do
    command -v "$cmd" >/dev/null 2>&1 || missing+=("$cmd")
done
if ((${#missing[@]})); then
    fail "missing programs: ${missing[*]}. On Debian or Ubuntu:
  sudo apt-get install git curl ca-certificates python3 python3-venv python3-dev build-essential ninja-build binutils-mips-linux-gnu"
fi
[[ "$(uname -m)" == "x86_64" ]] || fail "the IDO 5.3 build used here is for x86-64 Linux; this machine is $(uname -m)"

# --- 1. submodules ---------------------------------------------------------------------------------------------------
if [[ ! -f tools/splat/split.py || ! -f tools/asm-processor/build.py ]]; then
    [[ -e .git ]] || fail "tools/splat and tools/asm-processor are git submodules: clone the repository with git (--recursive)"
    say "== fetching the submodules (splat, asm-processor)"
    git submodule update --init
fi
if [[ -e .git ]] && git submodule status | grep -q '^[-+]'; then
    git submodule status >&2
    fail "a submodule is not at its pinned commit: run  git submodule update --init"
fi

# --- 2. Python packages ---------------------------------------------------------------------------------------------
if [[ ! -x .venv/bin/python3 ]]; then
    say "== creating the Python environment (.venv)"
    python3 -m venv .venv || fail "python3 -m venv failed: install python3-venv"
fi
say "== installing the pinned Python packages (requirements.txt)"
.venv/bin/python3 -m pip install --quiet --disable-pip-version-check --require-hashes -r requirements.txt \
    || fail "pip could not install requirements.txt (rabbitizer is built from source on Python 3.12 and later: install build-essential and python3-dev)"

# --- 3. IDO 5.3 --------------------------------------------------------------------------------------------------------
if sha256sum --quiet -c tools/checksums/ido-5.3.sha256 >/dev/null 2>&1; then
    say "== IDO 5.3: already in place and verified"
else
    say "== downloading IDO 5.3 (static recompilation by the decompals project, release 'per-function')"
    say "   $IDO_URL"
    tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' EXIT
    curl -fsSL --retry 3 -o "$tmp/ido.tar.gz" "$IDO_URL" || fail "download failed: $IDO_URL"
    echo "$IDO_TAR_SHA256  $tmp/ido.tar.gz" | sha256sum --quiet -c - || fail "the IDO download does not have the expected SHA-256"
    mkdir -p "$IDO_DIR"
    tar -xzf "$tmp/ido.tar.gz" -C "$IDO_DIR" --no-same-owner
    sha256sum --quiet -c tools/checksums/ido-5.3.sha256 || fail "an extracted IDO file does not match tools/checksums/ido-5.3.sha256"
    say "   verified: $(wc -l < tools/checksums/ido-5.3.sha256) files"
fi

# --- 4. SDK headers ------------------------------------------------------------------------------------------------------
n_hdr=$(wc -l < tools/checksums/sdk-2.0I.sha256)
if sha256sum --quiet -c tools/checksums/sdk-2.0I.sha256 >/dev/null 2>&1; then
    say "== SDK headers: in place and verified ($n_hdr files)"
elif [[ -n "$SDK_SRC" ]]; then
    [[ -f "$SDK_SRC/ultra64.h" && -d "$SDK_SRC/PR" ]] \
        || fail "$SDK_SRC is not a 2.0I include folder (it should hold ultra64.h and PR/)"
    say "== copying the libultra 2.0I SDK headers from $SDK_SRC"
    while read -r sum path; do
        rel="${path#include/2.0I/}"
        [[ -f "$SDK_SRC/$rel" ]] || fail "not in $SDK_SRC: $rel (tools/checksums/sdk-2.0I.sha256 lists the $n_hdr files needed)"
        mkdir -p "$(dirname "$path")"
        cp "$SDK_SRC/$rel" "$path"
    done < tools/checksums/sdk-2.0I.sha256
    sha256sum --quiet -c tools/checksums/sdk-2.0I.sha256 \
        || fail "a header does not match tools/checksums/sdk-2.0I.sha256 (another SDK version or an edited copy)"
    say "   verified: $n_hdr files"
else
    fail "the SDK headers in include/2.0I are missing or changed. They come with this repository:
  git checkout -- include/2.0I  restores them. Or run  tools/setup.sh --sdk-headers DIR  with DIR the 2.0I include
  folder of an SDK copy you have (the folder that holds ultra64.h and PR/). The $n_hdr files the build needs are
  listed, with their SHA-256, in tools/checksums/sdk-2.0I.sha256."
fi

say "setup OK. Next: put your ROM in place as baserom.jp.z64, then run  ./configure && ninja"
