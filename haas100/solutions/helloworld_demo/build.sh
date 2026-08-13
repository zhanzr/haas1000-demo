#!/usr/bin/env sh
# Build helper for the HaaS100 helloworld_demo app.
# Run from the solution directory. Requires the vendored SDK + toolchain.
set -e
case ":$PATH:" in *:/d/arm-none-eabi-tc/bin:*) ;;
  *) export PATH="/d/arm-none-eabi-tc/bin:$PATH" ;;
esac
scons --board=haas100 -j8
echo "Build OK: out/helloworld_demo@haas100.bin"
