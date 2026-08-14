#!/usr/bin/env bash
# Build the e_server reference backend for the host.
#   bash build.sh          # bundle web assets + compile e_server
#   ./e_server 8080        # run the reference backend
set -e
cd "$(dirname "$0")"

# 1. Bundle the web frontend into C arrays (inline CSS/JS, gzip, embed images)
python build_web.py

# 2. Compile the host reference backend
UNAME=$(uname -s 2>/dev/null || echo Windows)
case "$UNAME" in
  MINGW*|MSYS*|CYGWIN*) LIBS="-lws2_32" ;;
  *)                    LIBS="" ;;
esac

CC=""
for c in gcc clang cc; do
  if command -v "$c" >/dev/null 2>&1; then CC="$c"; break; fi
done
if [ -z "$CC" ] && [ -x /c/msys64/mingw64/bin/gcc.exe ]; then
  CC=/c/msys64/mingw64/bin/gcc.exe
  # gcc needs its own bin dir on PATH to find cc1/as/ld
  export PATH="/c/msys64/mingw64/bin:$PATH"
fi
if [ -z "$CC" ]; then
  echo "error: no C compiler found (gcc/clang/cc)" >&2
  exit 1
fi

echo "compiling with $CC"
$CC -O2 -Wall -Wextra -std=c11 -o e_server server.c $LIBS
echo "built ./e_server  (run: ./e_server 8080)"
