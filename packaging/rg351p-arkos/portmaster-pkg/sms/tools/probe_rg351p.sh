#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo "=== OS / ABI ==="
uname -a
cat /etc/os-release 2>/dev/null | head -n 6
ldd --version | head -n 1
strings /usr/lib/aarch64-linux-gnu/libstdc++.so.6 2>/dev/null | grep '^GLIBCXX_' | sort -V | tail -n 5

echo ""
echo "=== Memory / CMA ==="
free -m
dmesg 2>/dev/null | grep -i cma | tail -n 5

echo ""
echo "=== GPU / SDL2 Libraries ==="
ls -la /usr/local/lib/aarch64-linux-gnu/libmali* /usr/lib/aarch64-linux-gnu/libEGL* /usr/lib/aarch64-linux-gnu/libGLESv2* /usr/lib/aarch64-linux-gnu/libSDL2* 2>/dev/null

echo ""
echo "=== Dynamic link check for sms.aarch64 ==="
ldd "$SCRIPT_DIR/../sms.aarch64"

echo ""
echo "=== GLES / mmap Probe ==="
"$SCRIPT_DIR/probe_gles.aarch64"

echo ""
echo "=== sms_gx Headless Self-Test ==="
"$SCRIPT_DIR/gx_selftest.aarch64" --headless
