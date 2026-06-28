#!/usr/bin/env bash
#
# Regenerate the self-signed server cert/key and sync the firmware's embedded CA.
#
# The firmware embeds the server certificate (components/net/certs/server_ca.pem)
# and verifies the TLS connection against it. So whenever you regenerate the
# server cert here, you MUST rebuild & reflash the firmware.
#
# Usage:  ./make-certs.sh [SERVER_IP]      (default: 10.0.0.134)
#
set -euo pipefail

IP="${1:-10.0.0.134}"
DIR="$(cd "$(dirname "$0")" && pwd)"
CERT="$DIR/cert.pem"
KEY="$DIR/key.pem"
EMBED="$DIR/../components/net/certs/server_ca.pem"

openssl req -x509 -newkey rsa:2048 -nodes \
    -keyout "$KEY" -out "$CERT" -days 825 \
    -subj "/CN=$IP" -addext "subjectAltName=IP:$IP"

mkdir -p "$(dirname "$EMBED")"
cp "$CERT" "$EMBED"

echo
echo "Generated self-signed cert/key for ${IP}:"
echo "  server cert : ${CERT}"
echo "  server key  : ${KEY}    (private — gitignored, never commit)"
echo "  firmware CA : ${EMBED}  (committed, embedded via EMBED_TXTFILES)"
echo
echo "Next: rebuild & reflash the firmware so it trusts the new cert:"
echo "  idf.py build flash"
