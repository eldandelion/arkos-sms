# Super Mario Sunshine (PortMaster ARM64 / RG351P)

Native Linux `aarch64` build of [chasem-dev/sms-pc-port](https://github.com/chasem-dev/sms-pc-port) using its OpenGL ES 3.1+ backend adapted for Rockchip RK3326 / Mali-G31 handhelds (Anbernic RG351P on ArkOS).

## Notes

- **Upstream credit:** Built from `chasem-dev/sms-pc-port` (MIT License) and the `chasem-dev/sms-english` matching decompilation.
- **Required game data:** You must supply your own legally dumped North American (**GMSE01 Rev 0**) *Super Mario Sunshine* `.iso`, `.gcm`, or `.ciso` file and place it in `ports/sms/gamedata/`.
- **Hardware target:** Compiled with `-march=armv8-a -mtune=cortex-a35 -O3` and linked against `glibc 2.28` (`<= 2.30`) for full compatibility with ArkOS (Ubuntu 19.10) and ArkOS wuMMLe on RG351P.
- **Memory & performance tuning:** On 1 GB RAM devices, the launch script automatically sets `SMS_MEM_MB=24`, `SMS_GX_STREAM_MB=24`, `SMS_GX_ARENA_MB=16`, `SMS_GX_DL_CACHE_MB=24`, `SMS_GX_SCALE=0.50`, `SMS_GX_GL_THREAD=1`, disables MSAA/HD texture packs, and temporarily sets `performance` governors on CPU/DMC/GPU while playing.

## Controls (RG351P Digital Trigger Mapping)

Because the RG351P uses digital microswitches for L2/R2 instead of analog GameCube triggers, FLUDD's soft press and full press are split across `R1` and `R2` in `sms/conf/bindings.txt`:

| Handheld Button | GameCube Action | Function in Game |
| :--- | :--- | :--- |
| **Left Stick** | Control Stick | Move Mario |
| **Right Stick** | C-Stick | Rotate Camera |
| **D-Pad** | D-Pad | Camera / Menu navigation |
| **A / B / X / Y** | A / B / X / Y | Jump / Dive / Hover-Nozzle Switch / First-Person |
| **R1** | `R_SOFT` (40% Analog R) | Run & Spray FLUDD |
| **R2** | `R` (100% Full R) | Stand & Aim Spray |
| **L1** | `Z` | Area Map |
| **L2** | `L` | Center Camera / Ground Pound |
| **L3** | `L_SOFT` | Light L press |
| **Start** | `START` | Pause Menu |
| **Select + R1 / L1** | Save / Load State | Quick Save / Load State |
| **Select + Start** | PortMaster Hotkey | Exit Game |

## Compile

Built inside the `arkos-sms-builder:buster-arm64` Docker container (`arm64v8/debian:buster`, `glibc 2.28`, CMake 3.28.6, SDL 2.26.2 headers):

```bash
cmake -B build/linux-arm64 -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DSMS_GLES=ON \
  -DSMS_CPU_FLAGS="-march=armv8-a -mtune=cortex-a35 -O3"
cmake --build build/linux-arm64 --target sms -j4
```
