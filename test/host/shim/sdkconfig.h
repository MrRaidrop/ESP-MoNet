// SPDX-License-Identifier: MIT
// Host shim: empty sdkconfig. No CONFIG_MONET_CPP_LOGGER_BACKEND_USB defined,
// so logger_default_backend() selects the UART backend (the #else branch).
#ifndef SHIM_SDKCONFIG_H
#define SHIM_SDKCONFIG_H
#endif  // SHIM_SDKCONFIG_H
