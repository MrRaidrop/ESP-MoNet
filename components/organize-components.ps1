<#
.SYNOPSIS
    为 ESP‑IDF 项目下的 components/* 目录批量创建
    ├── src/
    ├── include/<comp_name>/
    └── CMakeLists.txt (如不存在)

.DESCRIPTION
    1. 搜索 components/ 下所有一级子目录 (即组件)
    2. 自动生成目录结构
    3. 可选：把散落在组件根目录的 .c / .h 文件移动到新目录
    4. 生成最小 CMakeLists.txt 模板，留空位给开发者后续补充
#>

$ErrorActionPreference = "Stop"

# 1. 找到 components 根路径
$componentsRoot = Join-Path $PSScriptRoot "components"
if (-not (Test-Path $componentsRoot)) {
    Write-Error "未找到 components 目录：$componentsRoot"
}

Get-ChildItem -Path $componentsRoot -Directory | ForEach-Object {

    $compDir  = $_.FullName            # 组件完整路径
    $compName = $_.Name                # 组件名

    Write-Host "`n▶  处理组件 [$compName] ..." -ForegroundColor Cyan

    # 2. 创建 src/ 与 include/<compName>/ 目录
    $srcDir = Join-Path $compDir "src"
    $incDir = Join-Path $compDir "include\$compName"

    foreach ($d in @($srcDir, $incDir)) {
        if (-not (Test-Path $d)) {
            New-Item -Path $d -ItemType Directory -Force | Out-Null
            Write-Host "    创建目录 $d"
        }
    }

    # ----- 可选：移动源码 -----
    # 把散落在组件根目录的 .c / .h 文件移到新目录（若你想手动整理则注释掉）
    Get-ChildItem -Path $compDir -File -Include *.c | ForEach-Object {
        Move-Item $_.FullName -Destination $srcDir -Force
        Write-Host "    移动  $($_.Name)  →  src/"
    }

    Get-ChildItem -Path $compDir -File -Include *.h | ForEach-Object {
        Move-Item $_.FullName -Destination $incDir -Force
        Write-Host "    移动  $($_.Name)  →  include/$compName/"
    }

    # 3. 生成最小 CMakeLists.txt（若不存在）
    $cmakeFile = Join-Path $compDir "CMakeLists.txt"
    if (-not (Test-Path $cmakeFile)) {

@"
#
# CMakeLists.txt for component: $compName
#

idf_component_register(
    SRCS
        # src/example.c
    INCLUDE_DIRS "include"
    # REQUIRES  driver nvs_flash  ...
)
"@ | Set-Content -Path $cmakeFile -Encoding UTF8

        Write-Host "    生成模板 CMakeLists.txt"
    }
}

Write-Host "`n✔  所有组件处理完毕！" -ForegroundColor Green
