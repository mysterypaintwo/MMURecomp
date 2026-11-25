#!/bin/sh
# Follow mmu_recomp's log on the Switch.
#   tools/recomp/watch_switch_log.sh <switch-ip>          live, while the game runs
#   tools/recomp/watch_switch_log.sh --ftp <ip:port> [log.txt|boot.txt]
#                                                         afterwards, from an FTP server
# Live mode connects to the log server built into mmu_recomp.nro (TCP 28280;
# recomp/src/log.cpp). It first receives the last 64 KiB, then every new line,
# and reconnects when the game is restarted.
set -u
if [ "${1:-}" = "--ftp" ]; then
    URL="ftp://${2:?ip:port}/switch/mmu_recomp/${3:-log.txt}"
    exec curl -s "$URL"
fi
IP=${1:?usage: watch_switch_log.sh <switch-ip> | --ftp <ip:port> [file]}
while :; do
    nc "$IP" 28280 2>/dev/null
    sleep 1
done
