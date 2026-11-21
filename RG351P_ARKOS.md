# Super Mario Sunshine — Anbernic RG351P / ArkOS PortMaster Adaptation

Fork/branch of [`chasem-dev/sms-pc-port`](https://github.com/chasem-dev/sms-pc-port) (`port/rg351p-arkos`, branched from `feat/android-arm64`) adding native Linux `aarch64` + OpenGL ES 3.1+ support, low-memory tuning, and PortMaster-MV packaging for Rockchip RK3326 / Mali-G31 handhelds such as the Anbernic RG351P running ArkOS.

## What Changed for RG351P / Linux ARM64

- **Build & ABI (`CMakeLists.txt`, `platform/gx/CMakeLists.txt`):**
  - Added Linux `aarch64` detection (`SMS_ARCH=64`, `SMS_RELOCATABLE_MEM1=ON`, `-march=armv8-a` default, optional `SMS_CPU_FLAGS` such as `-march=armv8-a -mtune=cortex-a35 -O3`).
  - Added `SMS_GLES` option (`SMS_GX_GLES=1`) to enable the GLES 3.1+ adapter (`gx_android.cpp`), Khronos headers, and `-O3` on `sms_gx` without requiring `__ANDROID__`.
- **Kernel & Memory Compatibility (`platform/port_runtime.cpp`, `decomp-patches/android-01..05`):**
  - Fallback when `MAP_FIXED_NOREPLACE` fails with `EINVAL`/`EOPNOTSUPP` on older BSP kernels (e.g. RK3326 Linux 4.4.189).
  - Decoupled `< 4 GiB` MEM1 relocation (`SMS_RELOCATABLE_MEM1`) and GLES pollution metric / shader fixes (`SMS_GX_GLES`) from `__ANDROID__`.
  - Added env overrides for 1 GB RAM budgets: `SMS_GX_STREAM_MB`, `SMS_GX_ARENA_MB`, `SMS_GX_DL_CACHE_MB`, and fractional `SMS_GX_SCALE` down to `0.50`.
- **Mali-G31 / RK3326 Performance Tweaks (`platform/gx/src/`):**
  - Enabled `SMS_GX_GL_THREAD` by default on non-Android Linux KMSDRM.
  - Added 1-entry uniform state cache and fast-path empty-sampler-bias skip in `gx_android.cpp`, and skipped redundant scissor testing on fullscreen `clearRect` passes in `gx_render.cpp`.
- **Docker Build Environment & PortMaster Package (`packaging/rg351p-arkos/`):**
  - `packaging/rg351p-arkos/docker/Dockerfile`: Debian Buster `arm64` image (`glibc 2.28`) matching ArkOS/PortMaster ABI, plus standalone GLES/SDL probe sources.
  - `packaging/rg351p-arkos/portmaster-pkg/`: PortMaster `port.json`, `gameinfo.xml`, launch script (`Super Mario Sunshine.sh`), RK3326 settings, and digital-trigger RG351P FLUDD bindings.

## Building & Packaging

See [packaging/rg351p-arkos/portmaster-pkg/README.md](packaging/rg351p-arkos/portmaster-pkg/README.md) for controls, runtime settings, and Docker build instructions:

```bash
docker build -t arkos-sms-builder:buster-arm64 packaging/rg351p-arkos/docker
docker run --rm -v "$PWD:/work" -w /work arkos-sms-builder:buster-arm64 bash -c '
  cmake -B build/linux-arm64 -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSMS_GLES=ON -DSMS_CPU_FLAGS="-march=armv8-a -mtune=cortex-a35 -O3" &&
  cmake --build build/linux-arm64 --target sms -j4 &&
  cp build/linux-arm64/sms build/linux-arm64/sms.aarch64 &&
  strip build/linux-arm64/sms.aarch64 &&
  python3 tools/savestate_ranges.py --map build/linux-arm64/sms.map --exe build/linux-arm64/sms.aarch64 --archive libsms_game.a --archive libsms_eclipse.a
'
```

## Disclaimer

This repository contains **no copyrighted game assets, ROMs, disc images, or save files**. You must supply your own legally dumped North American (**GMSE01 Rev 0**) *Super Mario Sunshine* `.iso`, `.gcm`, or `.ciso` in `ports/sms/gamedata/`.
