// https_post.h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 发�?JSON �?HTTPS 服务�?
 *
 * @param json 构建好的 JSON 字符�?
 */
void http_post_json(const char *json);

#ifdef __cplusplus
}
#endif
