#pragma once

#include "mongoose.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOGE(fmt, arg...) LOG(LL_ERROR, (fmt, ##arg))
#define LOGI(fmt, arg...) LOG(LL_INFO, (fmt, ##arg))
#define LOGD(fmt, arg...) LOG(LL_DEBUG, (fmt, ##arg))
#define LOGV(fmt, arg...) LOG(LL_VERBOSE_DEBUG, (fmt, ##arg))

#ifdef __cplusplus
}
#endif
