#!/bin/sh
# Build helper for the HaaS100 helloworld_demo app.
# Run from the solution directory (mingw bash / git-bash). Requires the
# vendored SDK + toolchain.
#
# NOTE: use `#!/bin/sh` (not `#!/usr/bin/env sh`) - the `env` wrapper
# strips Windows vars like PROCESSOR_ARCHITECTURE that scons needs.
set -e
case ":$PATH:" in *:/d/arm-none-eabi-tc/bin:*) ;;
  *) export PATH="/d/arm-none-eabi-tc/bin:$PATH" ;;
esac
[ -n "$PROCESSOR_ARCHITECTURE" ] || export PROCESSOR_ARCHITECTURE=AMD64

# The pip SCons is installed under the lowercase 'scons' package; bash's
# plain `python` may resolve to a store stub without it. Find a python
# that can import scons, then bootstrap SCons through it (same trick the
# aostools Make module uses).
run_scons() {
    py=""
    for cand in python python3; do
        if command -v "$cand" >/dev/null 2>&1 && "$cand" -c "import scons" >/dev/null 2>&1; then
            py="$cand"
            break
        fi
    done
    if [ -z "$py" ]; then
        for cand in /c/Users/*/AppData/Local/Programs/Python/Python*/python.exe \
                    /c/Program\ Files/Python*/python.exe; do
            if [ -x "$cand" ] && "$cand" -c "import scons" >/dev/null 2>&1; then
                py="$cand"
                break
            fi
        done
    fi
    if [ -z "$py" ]; then
        echo "error: no python with the scons module found" >&2
        exit 1
    fi
    "$py" -c "import scons, sys; sys.path.extend(scons.__path__); from SCons.Script import main; main()" "$@"
}

run_scons --board=haas100 -j8
echo "Build OK: out/helloworld_demo@haas100.bin"
