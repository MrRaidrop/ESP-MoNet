// https_post.h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 发送 JSON 到 HTTPS 服务器
 *
 * @param json 构建好的 JSON 字符串
 */
void http_post_json(const char *json);

#ifdef __cplusplus
}
#endif
