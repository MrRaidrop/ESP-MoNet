#!/bin/bash

# 源目录
SRC_DIR="./components"
# 目标目录
DEST_DIR="../backupCOMP"

# 创建目标目录（如果不存在）
mkdir -p "$DEST_DIR"

# 查找并复制 .c 文件
find "$SRC_DIR" -type f -name "*.c" | while read -r file; do
    echo "Copying: $file"
    cp "$file" "$DEST_DIR"
done

echo "✅ 所有 .c 文件已复制到 $DEST_DIR"

