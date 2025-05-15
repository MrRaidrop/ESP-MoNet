// include/server/file_store.h
#pragma once
#include <stddef.h>

#define FILE_STORE_MAX_BYTES   (200 * 1024 * 1024)   /* 200 MB quota */
#define FILE_STORE_DIR         "images"              /* relative to working dir */

int  file_store_init(void);                          /* mkdir + scan existing */
int  file_store_save(const void *buf, size_t len,
                     char *out_path, size_t out_len);/* auto-purge + save */
size_t file_store_total_bytes(void);                 /* current footprint */
