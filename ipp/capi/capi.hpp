#pragma once

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#define IVL_API_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define IVL_API_EXPORT
#endif
