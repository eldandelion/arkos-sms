# arkos-sms — Super Mario Sunshine (RG351P / ArkOS ARM64)

Native Linux `aarch64` + OpenGL ES 3.1+ port and **PortMaster** package of **Super Mario Sunshine** (`GMSE01` Rev 0) for the **Anbernic RG351P** (Rockchip RK3326: 4× Cortex-A35 @ 1.296 GHz, Mali-G31 MP2 GLES 3.2, 1 GB DDR3L RAM, 480×320 display) running **ArkOS**.

Adapted from [`chasem-dev/sms-pc-port`](https://github.com/chasem-dev/sms-pc-port) (`feat/android-arm64`) and the [`chasem-dev/sms-english`](https://github.com/chasem-dev/sms-english) matching decompilation.

> **Disclaimer:** **No copyrighted game data, disc images, ROMs, or save files are included.**
> You must supply your own legally dumped North American (**GMSE01 Rev 0**) `.iso`, `.gcm`, or `.ciso` image at runtime.

---

## Quick Install (PortMaster Package)

1. Download **`sms-rg351p-portmaster.zip`** from the [Releases page](https://github.com/eldandelion/arkos-sms/releases/latest).
2. Extract it into your device's `ports/` directory so you have:
   - `ports/Super Mario Sunshine.sh`
   - `ports/sms/`
3. Place your North American (**GMSE01 Rev 0**) *Super Mario Sunshine* `.iso`, `.gcm`, or `.ciso` inside `ports/sms/gamedata/`.
4. Launch **Super Mario Sunshine** from **Ports** in EmulationStation.

*(If your dump is in `.rvz` format, convert it to `.iso` first with Dolphin or `nodtool`.)*

---

## RG351P Controls

Because the RG351P has digital `L2`/`R2` switches instead of GameCube analog triggers, FLUDD's soft and full trigger presses are mapped separately in [`bindings.txt`](packaging/rg351p-arkos/portmaster-pkg/sms/conf/bindings.txt):

| RG351P Input | GameCube Action | Function in Game |
| :--- | :--- | :--- |
| **Left Stick** | Control Stick | Move Mario |
| **Right Stick** | C-Stick | Rotate Camera |
| **D-Pad** | D-Pad | Camera / Menu navigation |
| **A / B / X / Y** | A / B / X / Y | Jump / Dive / Hover-Nozzle / First-Person |
| **R1** | `R_SOFT` (40% Analog R) | Run & spray FLUDD without stopping |
| **R2** | `R` (100% Full R) | Stand & aim spray |
| **L1** | `Z` | Area Map |
| **L2** | `L` | Center Camera / Ground Pound |
| **L3** | `L_SOFT` | Light `L` press |
| **Start** | `START` | Pause Menu |
| **Select + R1 / L1** | Save / Load State | Quick Save / Quick Load |
| **Select + Start** | Hotkey | Exit Game |

---

## Performance & Hardware Notes (RG351P)

Out-of-the-box launcher defaults in [`Super Mario Sunshine.sh`](packaging/rg351p-arkos/portmaster-pkg/Super%20Mario%20Sunshine.sh) are tuned for 1 GB RAM and Mali-G31:

- `SMS_GX_SCALE=0.50`, `SMS_GX_GL_THREAD=1`, `SMS_VSYNC=off`, `SMS_MSAA=0`, `SMS_TEXTURE_PACKS=off`
- Low-RAM stream/arena/DL-cache budgets (`SMS_MEM_MB=24`, `SMS_GX_STREAM_MB=24`, `SMS_GX_ARENA_MB=16`, `SMS_GX_DL_CACHE_MB=24`)
- Temporarily sets CPU (`1.296 GHz`), DMC (`786 MHz`), and Mali GPU (`520 MHz`) governors to `performance` during gameplay and restores them on exit

**Measured performance on RG351P (`ArkOS wuMMLe`):**
- Menus and lighter areas can approach ~30 FPS.
- Heavy open areas such as **Delfino Plaza** run around **16–20 FPS (`~50–64 ms/frame`)** depending on camera draw count (`~420–780 draws/frame`).
- Consistent 30 FPS in Delfino Plaza is limited by single-core CPU game/display-list work (`~25–32 ms/frame` on Cortex-A35) and GLES driver submission overhead (`~2,100–3,500 GL calls/frame`).
- To trade frame rate for sharper visuals in lighter areas, adjust `SMS_GX_SCALE` (e.g., `0.60` or `0.75`) in `ports/Super Mario Sunshine.sh`.

---

## What Changed vs. Upstream (`chasem-dev/sms-pc-port`)

Branched from `feat/android-arm64` on [`port/rg351p-arkos`](RG351P_ARKOS.md):

- **Linux ARM64 & GLES 3.1+ build options ([`CMakeLists.txt`](CMakeLists.txt), [`platform/gx/CMakeLists.txt`](platform/gx/CMakeLists.txt)):**
  - Added Linux `aarch64` detection (`SMS_ARCH=64`, `SMS_RELOCATABLE_MEM1=ON`, `-march=armv8-a` default, `SMS_CPU_FLAGS` override) and `SMS_GLES` (`SMS_GX_GLES=1`) to compile the GLES 3.1+ backend on desktop/handheld Linux without `__ANDROID__`.
- **Kernel 4.4 & 64-bit Low-RAM Fixes ([`platform/port_runtime.cpp`](platform/port_runtime.cpp), [`decomp-patches/`](decomp-patches/)):**
  - Added fallback in `map_exact()` when `MAP_FIXED_NOREPLACE` returns `EINVAL`/`EOPNOTSUPP` on older RK3326 BSP kernels (`4.4.189`).
  - Decoupled `< 4 GiB` MEM1 relocation (`SMS_RELOCATABLE_MEM1`) and pollution-metric FBO / GLES shader guards (`SMS_GX_GLES`) from `__ANDROID__`.
  - Added env overrides for stream buffer, arena, DL cache, and fractional `SMS_GX_SCALE` down to `0.50`.
- **GLES / GL-Thread Optimizations (`platform/gx/src/`):**
  - Enabled `SMS_GX_GL_THREAD` by default on non-Android Linux KMSDRM ([`gx_glthread.cpp`](platform/gx/src/gx_glthread.cpp)).
  - Added fast-path uniform state caching ([`gx_android.cpp`](platform/gx/src/gx_android.cpp)) and skipped redundant scissor state on fullscreen clears ([`gx_render.cpp`](platform/gx/src/gx_render.cpp)).
- **Docker Build & PortMaster Packaging ([`packaging/rg351p-arkos/`](packaging/rg351p-arkos/)):**
  - Reproducible Debian Buster (`glibc 2.28`) ARM64 Docker toolchain, GLES/SDL KMSDRM diagnostic probes, and PortMaster launch/config assets.

---

## Building from Source

Requires Docker (running `linux/arm64` natively on Apple Silicon or via QEMU binfmt on x86_64):

```sh
git clone --recursive https://github.com/eldandelion/arkos-sms.git
cd arkos-sms

# 1. Build the Debian Buster ARM64 toolchain image (glibc 2.28 compatible with ArkOS)
docker build -t arkos-sms-builder:buster-arm64 packaging/rg351p-arkos/docker

# 2. Cross/container build sms.aarch64 tuned for RK3326 (Cortex-A35)
docker run --rm -v "$PWD:/work" -w /work arkos-sms-builder:buster-arm64 bash -c '
  cmake -B build/linux-arm64 -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DSMS_GLES=ON \
    -DSMS_CPU_FLAGS="-march=armv8-a -mtune=cortex-a35 -O3" &&
  cmake --build build/linux-arm64 --target sms -j4 &&
  cp build/linux-arm64/sms build/linux-arm64/sms.aarch64 &&
  strip build/linux-arm64/sms.aarch64 &&
  python3 tools/savestate_ranges.py \
    --map build/linux-arm64/sms.map \
    --exe build/linux-arm64/sms.aarch64 \
    --archive libsms_game.a \
    --archive libsms_eclipse.a
'
```

*(Run `savestate_ranges.py` after `strip` so the embedded SHA-256 fingerprint matches the stripped binary.)*

To assemble a PortMaster zip from `packaging/rg351p-arkos/portmaster-pkg/`, copy `build/linux-arm64/sms.aarch64` to `sms/sms.aarch64` and zip the folder contents cleanly.

---

## Repository Layout

```
packaging/rg351p-arkos/
  docker/             Debian Buster ARM64 Dockerfile + GLES/SDL probe sources
  portmaster-pkg/     PortMaster manifest, launch script, settings, bindings, tools
RG351P_ARKOS.md       Detailed porting & packaging notes
decomp/               Matching decompilation submodule (chasem-dev/sms-english)
decomp-patches/       Port patches applied at CMake configure time
platform/             Host replacements for GameCube GX, audio, disc, input, OS
src/                  Port entry point & compatibility headers
tools/                Savestate range stamping and helper scripts
docs/                 Upstream PC port documentation ([BUILD.md](BUILD.md), [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md), [docs/64-BIT.md](docs/64-BIT.md))
```

## Credits

- Upstream PC port: [`chasem-dev/sms-pc-port`](https://github.com/chasem-dev/sms-pc-port) (MIT License)
- Decompilation: [`chasem-dev/sms-english`](https://github.com/chasem-dev/sms-english)
