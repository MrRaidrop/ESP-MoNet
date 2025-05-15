/** 
 * @file log.h
 * @brief Logging macros for the server.
 */

#pragma once
#include <stdio.h>

#define LOGI(tag, fmt, ...)  printf("[I] %s: " fmt "\n", tag, ##__VA_ARGS__)
#define LOGE(tag, fmt, ...)  fprintf(stderr, "[E] %s: " fmt "\n", tag, ##__VA_ARGS__)
