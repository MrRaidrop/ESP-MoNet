# HTTPS Server (C + OpenSSL)

A lightweight custom HTTPS server implemented in C using OpenSSL. It serves firmware over secure HTTPS and logs incoming JSON data from ESP32 devices. Suitable for OTA and telemetry.

## Features

- Secure HTTPS communication with TLS 1.2+
- `GET /firmware.bin` for firmware download (supports HTTP Range)
- `GET /firmware.sha256` returns firmware SHA256 hash
- `POST /data` accepts and logs JSON payloads
- Multithreaded design, supports up to 10 concurrent clients
- Can run as a background service using `systemd`
- And sorry it is a little bit hard to read the code, I didn't make it modular because this project is not about the server.

## Directory Structure

```
/opt/myserver/
├── https_server          # Compiled HTTPS server binary
├── cert.pem              # Self-signed certificate
├── key.pem               # Private key
├── firmware.bin          # Firmware file to be served
├── https_data.log        # Log file for incoming JSON payloads
```

## Compilation

Install OpenSSL dev libraries:

```bash
sudo apt update
sudo apt install libssl-dev
```

Compile using:

```bash
gcc server.c -o https_server -lssl -lcrypto -lpthread
```

Place the executable and `firmware.bin` into `/opt/myserver/`.

## Running with systemd

Create `/etc/systemd/system/my_https_server.service`:

```ini
[Unit]
Description=My HTTPS OTA Server
After=network.target

[Service]
ExecStart=/opt/myserver/https_server
WorkingDirectory=/opt/myserver
Restart=on-failure
RestartSec=3
Type=simple
User=www-data
Group=www-data
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

> If your certificate path differs, modify:

```c
SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM);
SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM);
```

Then reload and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable my_https_server.service
sudo systemctl start my_https_server.service
```

## API Endpoints

### `GET /firmware.bin`

Download firmware with optional Range:

```bash
curl -k --http1.1 https://your.server.ip:8443/firmware.bin -o firmware_out.bin
```

Partial download:

```bash
curl -k --http1.1 https://your.server.ip:8443/firmware.bin -H "Range: bytes=0-1023" -o partial.bin
```

### `GET /firmware.sha256`

Returns SHA256 hash of `firmware.bin`:

```bash
curl -k https://your.server.ip:8443/firmware.sha256
```

### `POST /data`

Send JSON data:

```bash
curl -k https://your.server.ip:8443/data      -H "Content-Type: application/json"      -d '{"light": 895, "ts": "2025-04-12T14:23:01"}'
```

Will be appended to `https_data.log`.

### `GET /status`

Returns server uptime in seconds:

```json
{"uptime": 12345}
```

## Log Viewing

```bash
tail -f /opt/myserver/https_data.log
```

Or via journalctl:

```bash
sudo journalctl -u my_https_server.service --since "10 minutes ago"
```

## Security Notes

- For production, use certificates from a trusted CA
- Certificates must be readable by the service user
- Server handles one request per connection
- Supports standard Range headers for OTA

## License

MIT License — Free to use and modify.