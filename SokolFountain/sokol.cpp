#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_fetch.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include <sokol_time.h>
#define SOKOL_DEBUGTEXT_IMPL
#include "util/sokol_debugtext.h"

#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_CPP_MODE
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"