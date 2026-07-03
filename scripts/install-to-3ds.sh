#!/usr/bin/env bash
# Build + install 3DS2SPOUT on SD via WiFi (FTP + optional 3dslink).
# Usage: ./scripts/install-to-3ds.sh 192.168.1.45
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP_DIR="$ROOT/3ds-cam-stream"
THREEDS_IP="${1:-}"
PC_PORT="${PC_PORT:-5000}"
FTP_PORT="${FTP_PORT:-5000}"
THREE_DS_LINK_PORT="${THREE_DS_LINK_PORT:-17491}"
FTP_USER="${FTP_USER:-anonymous}"
FTP_PASS="${FTP_PASS:-}"
SKIP_BUILD="${SKIP_BUILD:-0}"
SKIP_FTP="${SKIP_FTP:-0}"
SKIP_LAUNCH="${SKIP_LAUNCH:-0}"
CURL_OPTS=(--connect-timeout 15 --max-time 120 -sf)

usage() {
    echo "Usage: $0 <3DS_IP> [PC_IP]"
    echo "  Env: FTP_PORT FTP_USER FTP_PASS SKIP_BUILD SKIP_FTP SKIP_LAUNCH"
    exit 1
}

[[ -n "$THREEDS_IP" ]] || usage

detect_pc_ip() {
    if [[ -n "${2:-}" ]]; then
        echo "$2"
        return
    fi
    if command -v ipconfig >/dev/null 2>&1; then
        ipconfig | grep -E "IPv4" | head -1 | sed 's/.*: *//'
        return
    fi
    hostname -I 2>/dev/null | awk '{print $1}'
}

PC_IP="$(detect_pc_ip "$@")"
[[ -n "$PC_IP" ]] || { echo "Could not detect PC IP; pass as second argument."; exit 1; }

echo "=== 3DS2SPOUT — install sur SD via WiFi ==="
echo "PC IP: $PC_IP:$PC_PORT"
echo "3DS IP: $THREEDS_IP"

if [[ "$SKIP_BUILD" != "1" ]]; then
    echo "Building..."
    make -C "$APP_DIR" clean 2>/dev/null || true
    make -C "$APP_DIR" PC_IP="$PC_IP" PC_PORT="$PC_PORT"
fi

THREEDSX="$APP_DIR/3ds-cam-stream.3dsx"
CFG="$(mktemp)"
trap 'rm -f "$CFG"' EXIT
printf 'pc_ip=%s\npc_port=%s\n' "$PC_IP" "$PC_PORT" >"$CFG"

if [[ "$SKIP_FTP" != "1" ]]; then
    echo "FTP upload to ${THREEDS_IP}:${FTP_PORT} ..."
    echo "  (Lance FTPD sur la 3DS. Port 17491 = 3dslink, pas FTP -> FTP_PORT=5000 ou ton port FTPD.)"
    if curl "${CURL_OPTS[@]}" --ftp-create-dirs -T "$THREEDSX" \
        "ftp://${THREEDS_IP}:${FTP_PORT}/3ds/3ds-cam-stream.3dsx" \
        --user "${FTP_USER}:${FTP_PASS}"; then
        curl "${CURL_OPTS[@]}" -T "$CFG" \
            "ftp://${THREEDS_IP}:${FTP_PORT}/3ds-cam-stream.cfg" \
            --user "${FTP_USER}:${FTP_PASS}"
        echo "Installed on SD: sdmc:/3ds/3ds-cam-stream.3dsx + sdmc:/3ds-cam-stream.cfg"
    else
        echo "FTP failed — start FTPD on the 3DS or: SKIP_FTP=1 $0 $*"
        echo "  Try: FTP_PORT=17491 or FTP_PASS=1234"
    fi
fi

if [[ "$SKIP_LAUNCH" != "1" ]] && command -v 3dslink >/dev/null; then
    echo "Launching via 3dslink (NetLoader port 17491 — press Y in Homebrew Launcher first)..."
    3dslink -a "$THREEDS_IP" "$THREEDSX" || true
fi

echo "Done."
