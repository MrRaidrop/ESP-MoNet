#!/usr/bin/env bash
#
# One-time setup for ESP-MoNet.
#
# Prereqs: ESP-IDF v5.4.x installed and sourced in this shell:
#     . $IDF_PATH/export.sh
#
# Usage:
#     ./setup.sh [SERVER_IP]
#
# Pass your server's LAN IP to auto-generate its TLS cert and point the
# firmware at it. Without it, the defaults from sdkconfig.default are used and
# you can edit the URLs in sdkconfig afterwards.
#
set -euo pipefail
cd "$(dirname "$0")"

SERVER_IP="${1:-}"

# 1. ESP-IDF available?
if ! command -v idf.py >/dev/null 2>&1; then
    echo "ERROR: idf.py not found. Install ESP-IDF v5.4.x, then:"
    echo "         . \$IDF_PATH/export.sh"
    exit 1
fi

# 2. Server cert (the firmware embeds it; the server presents it). Self-signed.
if [ -n "$SERVER_IP" ]; then
    echo "==> Generating server cert/key for $SERVER_IP"
    ./server/make-certs.sh "$SERVER_IP"
elif [ ! -f components/net/certs/server_ca.pem ]; then
    echo "==> Generating a default server cert (10.0.0.134); re-run with your IP to change"
    ./server/make-certs.sh
fi

# 3. Apply project defaults and set the target.
echo "==> Configuring target esp32s3 from sdkconfig.default"
rm -f sdkconfig
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.default" set-target esp32s3

# 4. Patch the server IP into the generated sdkconfig if provided.
if [ -n "$SERVER_IP" ]; then
    sed -i "s#https://[^:\"]*:8443/data#https://$SERVER_IP:8443/data#"  sdkconfig
    sed -i "s#https://[^:\"]*:8443/image#https://$SERVER_IP:8443/image#" sdkconfig
fi

echo
echo "================================================================"
echo " Setup complete."
echo
echo " 1. Set your Wi-Fi credentials in sdkconfig (do NOT commit them):"
echo "      CONFIG_WIFI_SSID / CONFIG_WIFI_PASSWORD"
if [ -z "$SERVER_IP" ]; then
    echo "    and your server URLs:"
    echo "      CONFIG_HTTPS_SERVER_URL / CONFIG_HTTPS_IMAGE_URL"
fi
echo " 2. Build & flash:   idf.py -p <PORT> build flash monitor"
echo " 3. Live camera:     open http://<board-ip>/   (IP is in the boot log)"
echo " 4. Run the server:  cd server && ./make-certs.sh <SERVER_IP> && \\"
echo "                     mkdir -p build && cd build && cmake .. && make -j && ./https_server"
echo "================================================================"
