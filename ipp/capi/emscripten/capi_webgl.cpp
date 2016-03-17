#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <html5.h>

extern "C" {

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE EMSCRIPTEN_KEEPALIVE gl_context_initialize(const char* target)
{
    EmscriptenWebGLContextAttributes attribs;
    emscripten_webgl_init_context_attributes(&attribs);
    auto handle = emscripten_webgl_create_context(target, &attribs);
    return handle;
}

void EMSCRIPTEN_KEEPALIVE gl_context_activate(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE handle)
{
    emscripten_webgl_make_context_current(handle);
}
}
#endif
