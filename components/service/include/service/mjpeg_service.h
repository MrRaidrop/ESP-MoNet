#ifndef MJPEG_SERVICE_H_
#define MJPEG_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "monet_core/service_registry.h"

/**
 * @file mjpeg_service.h
 * @brief Live MJPEG stream server.
 *
 * Owns the camera and runs an esp_http_server on port 80 serving:
 *   GET /         -> minimal HTML page that embeds the stream
 *   GET /stream   -> multipart/x-mixed-replace MJPEG (one persistent connection)
 *
 * Open http://<board-ip>/ in a browser for live video. Because frames are
 * pushed over a single kept-open connection there is no per-frame TLS/TCP
 * handshake (the bottleneck of the per-frame upload path).
 *
 * NOTE: this service owns the camera directly, so do NOT also register
 * camera_service (the per-frame uploader) — only one camera owner is allowed.
 */
const service_desc_t* get_mjpeg_service(void);

#ifdef __cplusplus
}
#endif

#endif // MJPEG_SERVICE_H_
