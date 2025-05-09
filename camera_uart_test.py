import struct
import serial
import time
import zlib
from datetime import datetime
import os

# 配置参数
PORT = "COM8"                # 串口名
BAUD = 921600                # 波特率
MAGIC = b'\xA5\xA5\xA5\xA5'  # 帧起始标记
HEADER_SIZE = 16             # 帧头长度
BUFFER_LIMIT = 2 * 1024 * 1024  # 最大缓存 2MB
SAVE_BAD_FRAMES = False      # 是否保存 CRC 错误的帧

def log(msg, level="info"):
    prefix = {
        "info": "[INFO]",
        "warn": "\033[33m[WARN]\033[0m",
        "error": "\033[31m[ERROR]\033[0m"
    }.get(level, "[INFO]")
    print(f"{prefix} {msg}")

def save_jpeg(filename, data):
    with open(filename, "wb") as f:
        f.write(data)
    log(f"[SUCCESS] Frame saved: {filename}")

def main():
    print(f"[INIT] Opening {PORT} @ {BAUD}...")
    try:
        ser = serial.Serial(PORT, baudrate=BAUD, timeout=0.2)
    except Exception as e:
        log(f"Failed to open serial: {e}", "error")
        return

    buffer = b""
    frame_id = 0

    while True:
        chunk = ser.read(8192)
        if chunk:
            buffer += chunk

        while True:
            idx = buffer.find(MAGIC)
            if idx == -1:
                break

            # 检查是否有足够帧头
            if len(buffer) < idx + HEADER_SIZE:
                break

            # 解析帧头
            try:
                header = buffer[idx:idx + HEADER_SIZE]
                magic, w, h, length, crc = struct.unpack("<IHHII", header)
            except struct.error:
                log("Failed to unpack header", "error")
                buffer = buffer[idx + 1:]
                continue

            if length > 1024 * 1024:
                log(f"Invalid length: {length} bytes", "error")
                buffer = buffer[idx + 1:]
                continue

            total_len = HEADER_SIZE + length
            if len(buffer) < idx + total_len:
                # 不够数据，等待下个 chunk
                break

            data = buffer[idx + HEADER_SIZE: idx + total_len]
            calc_crc = zlib.crc32(data) & 0xFFFFFFFF

            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            name_base = f"frame_{frame_id:05d}_{timestamp}"

            if calc_crc == crc:
                filename = f"{name_base}.jpg"
                save_jpeg(filename, data)
            else:
                log(f"[ERROR] CRC mismatch! Expected 0x{crc:08X}, got 0x{calc_crc:08X}", "warn")
                if SAVE_BAD_FRAMES:
                    filename = f"{name_base}_bad.jpg"
                    save_jpeg(filename, data)

            # 跳过本帧
            buffer = buffer[idx + total_len:]
            frame_id += 1

        # 控制缓存大小，避免爆内存
        if len(buffer) > BUFFER_LIMIT:
            buffer = buffer[-1024:]
            log("Trimmed input buffer", "warn")

        time.sleep(0.01)

if __name__ == "__main__":
    os.makedirs("frames", exist_ok=True)
    os.chdir("frames")
    main()
