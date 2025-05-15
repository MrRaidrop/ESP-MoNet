
# curl -k -v https://localhost:8443/firmware.bin -o /dev/null  # 全量 200
# curl -k -H "Range: bytes=1000-" https://localhost:8443/firmware.bin -o /dev/null  # 断点续传 206
# curl -k -X POST https://localhost:8443/data -d '{"foo":42}'

#!/usr/bin/env bash
set -e
IMG=test.jpg                       # 准备一张小 JPEG
curl -k -v https://localhost:8443/image \
     -H "Content-Type: image/jpeg" \
     --data-binary @"$IMG"