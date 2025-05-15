# Lightweight HTTPS Server for ESP32-S3 Projects

A tiny C11 + OpenSSL HTTPS server designed to work out‑of‑the‑box with ESP‑MoNet / ESP32‑S3 firmware.  
It is **single‑binary, thread‑per‑connection**, and fully self‑contained (no external HTTP libraries).

## Core Features

| Endpoint | Method | Purpose | Notes |
|----------|--------|---------|-------|
| `/status` | **GET** | Returns server uptime in seconds. | JSON `{ "uptime": N }` |
| `/data`   | **POST** | Accepts JSON telemetry from ESP32. | Echoes `{ "result":"ok" }`. |
| `/firmware.bin` | **GET** | OTA firmware download. | Supports `Range: bytes=<start>-`, returns `206 Partial Content`. |
| `/firmware.sha256` | **GET** | Hex SHA‑256 of current firmware. | 64 hex chars. |
| `/image`  | **POST** | Upload JPEG from OV2640 (QVGA). | Saves under `images/` (200 MB rolling quota) and returns `201 Created` + `Location:`. |

```
images/
└─ 20250515_031259_1804289383.jpg
```

## Project Layout

```
server/
├── include/server/    # public headers
│   ├── handlers.h     # handler API + http_request_t
│   ├── router.h
│   ├── file_store.h
│   └── log.h
├── src/
│   ├── net/ssl_server.c
│   ├── router.c
│   ├── handlers/      # one .c per endpoint
│   └── storage/file_store.c
└── tests/             # curl smoke‑tests
```

## Build & Run

```bash
mkdir build && cd build
cmake ..
make -j
# copy self‑signed cert/key or point to your own in main.c
cp ../cert.pem ../key.pem .
./https_server &
```

You can see my bash script on auto_build.sh

All dependencies are standard on Ubuntu:

```bash
sudo apt install build-essential cmake libssl-dev
```

## Quick Tests

```bash
curl -k https://localhost:8443/status
curl -k -X POST https://localhost:8443/data      -H "Content-Type: application/json"      -d '{"type":"light","value":123}'
curl -k -o fw.bin https://localhost:8443/firmware.bin
curl -k -H "Range: bytes=1000-" -o part.bin https://localhost:8443/firmware.bin
curl -k --data-binary @test.jpg -H "Content-Type: image/jpeg"      https://localhost:8443/image
```

You can change localhost to your web server port.
By the way Oracle cloud offers free server, I'm using it, a little bit light, but enough in this case.

## Adding a New Handler

1. **Create file** `src/handlers/foo_handler.c`.
2. Implement the unified signature:

```c
int foo_handler(SSL *ssl, const http_request_t *req) {
    const char *resp = "HTTP/1.1 200 OK\r\n\r\nHello";
    return SSL_write(ssl, resp, strlen(resp));
}
```

3. **Declare** in `include/server/handlers.h`.
4. **Register** in `src/router.c`:

```c
{"GET", "/foo", foo_handler},
```

Re‑build and your endpoint is live.
To be honest, it should be enough for the data handling of Iot devices.

## Config Tuning

- **Port / cert / key / max_clients** → `server_config_t` in `main.c`.
- **Image quota** → `FILE_STORE_MAX_BYTES` in `file_store.h`.

## License

MIT (see `LICENSE`).

---

*Let's go~*
