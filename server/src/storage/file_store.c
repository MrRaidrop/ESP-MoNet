/* src/storage/file_store.c */
#include "server/file_store.h"
#include "server/log.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define TAG "FILE_STORE"

/* ---------------- 内部工具 ---------------- */
static size_t s_total = 0;

static int ensure_dir(void)
{
    struct stat st;
    if (stat(FILE_STORE_DIR, &st) == 0 && S_ISDIR(st.st_mode)) return 0;
    if (mkdir(FILE_STORE_DIR, 0755) == 0) return 0;
    LOGE(TAG, "mkdir %s failed: %s", FILE_STORE_DIR, strerror(errno));
    return -1;
}

static int scan_total(void)
{
    DIR *d = opendir(FILE_STORE_DIR);
    if (!d) return -1;
    size_t total = 0;
    struct dirent *de;
    struct stat st;
    char path[256];
    while ((de = readdir(d))) {
        if (de->d_type != DT_REG) continue;
        snprintf(path, sizeof(path), "%s/%s", FILE_STORE_DIR, de->d_name);
        if (stat(path, &st) == 0) total += st.st_size;
    }
    closedir(d);
    s_total = total;
    return 0;
}

/* 删除最旧文件直到足够空间 */
static void purge_if_needed(size_t need_bytes)
{
    if (s_total + need_bytes <= FILE_STORE_MAX_BYTES) return;

    DIR *d = opendir(FILE_STORE_DIR);
    if (!d) return;

    /* 先一次性列出 (文件名,mtime,size)，然后按 mtime 升序删 */
    typedef struct { char name[256]; time_t mtime; size_t size; } entry_t;
    entry_t *list = NULL; size_t cnt = 0;
    struct dirent *de; struct stat st; char path[256];

    while ((de = readdir(d))) {
        if (de->d_type != DT_REG) continue;
        snprintf(path, sizeof(path), "%s/%s", FILE_STORE_DIR, de->d_name);
        if (stat(path, &st) == 0) {
            list = realloc(list, (cnt+1)*sizeof(*list));
            strcpy(list[cnt].name, de->d_name);
            list[cnt].mtime = st.st_mtime;
            list[cnt].size  = st.st_size;
            ++cnt;
        }
    }
    closedir(d);
    /* 冒泡 / 选择排序都行，这里用 qsort */
    int cmp(const void *a,const void *b){
        return ((entry_t*)a)->mtime - ((entry_t*)b)->mtime;
    }
    qsort(list, cnt, sizeof(*list), cmp);

    for (size_t i = 0; i < cnt && s_total + need_bytes > FILE_STORE_MAX_BYTES; ++i) {
        snprintf(path, sizeof(path), "%s/%s", FILE_STORE_DIR, list[i].name);
        if (unlink(path) == 0) {
            s_total -= list[i].size;
            LOGI(TAG, "Purged %s (%zu bytes)", list[i].name, list[i].size);
        }
    }
    free(list);
}

/* ---------------- 对外接口 ---------------- */
int file_store_init(void)
{
    if (ensure_dir() < 0) return -1;
    if (scan_total() < 0) return -1;
    LOGI(TAG, "Init OK, current usage = %zu bytes", s_total);
    return 0;
}

size_t file_store_total_bytes(void) { return s_total; }

int file_store_save(const void *buf, size_t len,
                    char *out_path, size_t out_len)
{
    if (!buf || len == 0) return -1;
    purge_if_needed(len);

    /* 生成文件名：YYYYMMDD_HHMMSS_xxx.jpg */
    time_t t = time(NULL); struct tm tm; localtime_r(&t, &tm);
    char fname[64];
    snprintf(fname, sizeof(fname), "%04d%02d%02d_%02d%02d%02d_%ld.jpg",
             tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, random());

    char full[256];
    snprintf(full, sizeof(full), "%s/%s", FILE_STORE_DIR, fname);

    FILE *fp = fopen(full, "wb");
    if (!fp) { LOGE(TAG, "open %s fail: %s", full, strerror(errno)); return -1; }
    size_t written = fwrite(buf, 1, len, fp);
    fclose(fp);
    if (written != len) { unlink(full); return -1; }

    s_total += written;
    LOGI(TAG, "Saved %s (%zu B), total=%zu B", fname, written, s_total);

    if (out_path && out_len) strncpy(out_path, fname, out_len);
    return 0;
}
