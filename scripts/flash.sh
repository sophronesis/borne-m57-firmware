#!/usr/bin/env bash
# watches for Adafruit PlumBootloader to appear, then flashes the given uf2
# usage: flash.sh [path_to_uf2]
# defaults to firmware/borne_m57_via_fixed_ledconfig.uf2 in this repo

set -euo pipefail

UF2="${1:-$(dirname "$0")/../firmware/borne_m57_via_fixed_ledconfig.uf2}"
UF2="$(realpath "$UF2")"

if [[ ! -f "$UF2" ]]; then
    echo "uf2 not found: $UF2" >&2
    exit 1
fi

echo "watching for PlumBootloader. tweezer-double-tap reset on the half you want to flash..."
echo "uf2: $UF2 ($(stat -c%s "$UF2") bytes)"

while true; do
    if lsusb 2>/dev/null | grep -q "239a:005d"; then
        echo "bootloader detected, finding block device..."
        sleep 1
        DEV="$(lsblk -ndo NAME,LABEL 2>/dev/null | awk '$2 == "STM32F4Plum" {print "/dev/"$1; exit}')"
        if [[ -z "$DEV" ]]; then
            echo "block device not ready, retrying..." >&2
            sleep 1
            continue
        fi
        echo "found $DEV, mounting and flashing..."
        MNT="$(mktemp -d)"
        sudo mount "$DEV" "$MNT"
        sudo cp "$UF2" "$MNT/"
        sync; sync
        # unmount immediately so the bootloader can cleanly cycle to firmware
        sudo umount "$MNT" 2>/dev/null || true
        rmdir "$MNT" 2>/dev/null || true
        echo "waiting for bootloader to write firmware and reboot..."
        # poll: wait until firmware shows up (bootloader rewrites flash + resets)
        for i in $(seq 1 30); do
            if lsusb 2>/dev/null | grep -q "6401:45d4"; then
                echo "rebooted as firmware. done."
                exit 0
            fi
            sleep 1
        done
        if lsusb 2>/dev/null | grep -q "239a:005d"; then
            echo "still in bootloader after 30s - flash may have failed"
        else
            echo "didn't see firmware reboot - check the keyboard"
        fi
        exit 1
    fi
    sleep 1
done
