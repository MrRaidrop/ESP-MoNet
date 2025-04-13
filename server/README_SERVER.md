## 🌐 HTTPS Server (C + OpenSSL)

This is a lightweight custom HTTPS server implemented in C using OpenSSL. It securely receives JSON data from an ESP32 device via HTTPS POST and logs the received payloads to a local file for later processing.

---

### ✅ Features

- 🔒 Secure HTTPS server (TLS 1.2+) using self-signed certificates
- 📥 Handles HTTP `POST /data` with JSON payloads
- 📝 Appends received JSON to a local log file
- 📡 Accepts up to 5 concurrent clients
- 🛠️ Managed via `systemd` as a background Linux service

---

### 🗂 Directory Structure

```
/opt/myserver/
├── https_server         # Compiled C HTTPS server binary
├── cert.pem             # Self-signed certificate
├── key.pem              # Private key
├── https_data.log       # Logged POST data (auto-created)
```

---

### 🔧 Running as a Service

The server is launched and monitored using a `systemd` unit:

Please Change this two line of code if you keys are in different directory

SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0 ||
SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0)

```ini
# /etc/systemd/system/my_https_server.service

[Unit]
Description=🧪 My HTTP Server
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

After defining the unit, enable and start the service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable my_https_server.service
sudo systemctl start my_https_server.service
```

---

### 📋 View Logs

You can view incoming POST data in real time:

```bash
tail -f /opt/myserver/https_data.log
```

Or via system journal:

```bash
sudo journalctl -u my_https_server.service --since "10 minutes ago"
```

---

### 📤 Example POST Request

ESP32 or `curl` can send JSON like this:

```bash
curl -k https://your.server.ip:8443/data \
     -H "Content-Type: application/json" \
     -d '{"light": 895, "ts": "2025-04-12T14:23:01"}'
```

Server response:

```json
{"result": "received"}
```

---

### 🚨 Notes

- Certificates must be readable by the specified `User` in the service file (e.g., `www-data`)
- Server must bind to `0.0.0.0:8443` to accept ESP32 or external device connections
- Server handles one request per connection (simple stateless design)

---

### 🔐 Security Considerations

- For development, certificate verification is skipped by ESP32 (`cert_pem = NULL`)
- In production, consider using Let's Encrypt and enabling full chain validation
- Avoid storing plaintext keys with broad permissions

---

### 📜 License

MIT License — free to use, adapt, and deploy.

