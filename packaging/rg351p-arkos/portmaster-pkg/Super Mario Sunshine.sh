#!/bin/bash

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
get_controls

GAMEDIR="/$directory/ports/sms"
cd "$GAMEDIR"
> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1

export LD_LIBRARY_PATH="$GAMEDIR/libs.${DEVICE_ARCH}:$LD_LIBRARY_PATH"
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

# Locate user-supplied GMSE01 Rev 0 disc image (.iso, .gcm, or .ciso)
DISC_FILE=$(find "$GAMEDIR/gamedata" -maxdepth 1 -type f \( -iname "*.iso" -o -iname "*.gcm" -o -iname "*.ciso" \) | head -n 1)
if [ -z "$DISC_FILE" ]; then
  pm_show_error "Missing game disc! Place your North American (GMSE01 Rev 0) Super Mario Sunshine .iso, .gcm, or .ciso in ports/sms/gamedata/." --title "Super Mario Sunshine" --type warning
  exit 1
fi

# Keep saves, save states, and shader cache in $GAMEDIR/conf without pointing
# $HOME or $XDG_CONFIG_HOME at exFAT (PulseAudio/SDL break on FAT symlinks).
mkdir -p "$GAMEDIR/conf/saves/card-a" "$GAMEDIR/conf/savestates" "$GAMEDIR/conf/shadercache"
export SMS_SETTINGS="$GAMEDIR/conf/settings.txt"
export SMS_BINDINGS="$GAMEDIR/conf/bindings.txt"
export SMS_SAVE_DIR="$GAMEDIR/conf/saves/card-a"
export SMS_SAVE_STATE_DIR="$GAMEDIR/conf/savestates"
export SMS_GX_SHADER_CACHE="$GAMEDIR/conf/shadercache/gx-programs-64.bin"

# Fullscreen display matching device aspect/resolution
export SMS_FULLSCREEN=on
export SMS_VSYNC=off

# Tune memory & GPU budgets for 1 GB RAM / low-res handhelds (Anbernic RG351P / RK3326)
if [ "${DEVICE_RAM:-1}" -le 1 ]; then
  export SMS_MEM_MB="${SMS_MEM_MB:-24}"
  export SMS_GX_STREAM_MB="${SMS_GX_STREAM_MB:-24}"
  export SMS_GX_ARENA_MB="${SMS_GX_ARENA_MB:-16}"
  export SMS_GX_DL_CACHE_MB="${SMS_GX_DL_CACHE_MB:-24}"
  export SMS_GX_SCALE="${SMS_GX_SCALE:-0.50}"
  export SMS_GX_GL_THREAD="${SMS_GX_GL_THREAD:-1}"
  export SMS_MSAA=0
  export SMS_TEXTURE_PACKS=off
  export SMS_TEXTURE_PACK_MB=0
  export SMS_TEXTURE_PACK_PENDING_MB=0
fi

# Lock CPU, DDR3L DMC (528->786 MHz), and Mali GPU (480->520 MHz) to performance while running
PREV_CPU_GOV=""
PREV_DMC_GOV=""
PREV_GPU_GOV=""
if [ -f /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ]; then
  PREV_CPU_GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null)
  echo performance | $ESUDO tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor > /dev/null 2>&1 || true
fi
if [ -f /sys/class/devfreq/dmc/governor ]; then
  PREV_DMC_GOV=$(cat /sys/class/devfreq/dmc/governor 2>/dev/null)
  echo performance | $ESUDO tee /sys/class/devfreq/dmc/governor > /dev/null 2>&1 || true
fi
if [ -f /sys/class/devfreq/ff400000.gpu/governor ]; then
  PREV_GPU_GOV=$(cat /sys/class/devfreq/ff400000.gpu/governor 2>/dev/null)
  echo performance | $ESUDO tee /sys/class/devfreq/ff400000.gpu/governor > /dev/null 2>&1 || true
fi

$GPTOKEYB2 "sms.${DEVICE_ARCH}" > /dev/null 2>&1 &
pm_platform_helper "$GAMEDIR/sms.${DEVICE_ARCH}"
./sms.${DEVICE_ARCH} "$DISC_FILE"

[ -n "$PREV_CPU_GOV" ] && echo "$PREV_CPU_GOV" | $ESUDO tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor > /dev/null 2>&1 || true
[ -n "$PREV_DMC_GOV" ] && echo "$PREV_DMC_GOV" | $ESUDO tee /sys/class/devfreq/dmc/governor > /dev/null 2>&1 || true
[ -n "$PREV_GPU_GOV" ] && echo "$PREV_GPU_GOV" | $ESUDO tee /sys/class/devfreq/ff400000.gpu/governor > /dev/null 2>&1 || true

pm_finish
