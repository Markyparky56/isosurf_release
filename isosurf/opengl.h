#pragma once

#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined (DELETE)
#undef DELETE
#endif
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"

void initOpenglFuncs();

extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
