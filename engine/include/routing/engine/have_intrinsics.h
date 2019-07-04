
#include <absl/container/internal/have_sse.h>

#ifndef RA_ENGINE_HAVE_INTRINSICS_H
#define RA_ENGINE_HAVE_INTRINSICS_H

#ifndef RA_HAVE_SSE2
#ifdef SWISSTABLE_HAVE_SSE2
#define RA_HAVE_SSE2 1
#else
#define RA_HAVE_SSE2 0
#endif
#endif

#ifndef RA_HAVE_SSE3
#ifdef SWISSTABLE_HAVE_SSE3
#define RA_HAVE_SSE3 1
#else
#define RA_HAVE_SSE3 0
#endif
#endif

#ifndef RA_HAVE_AVX
#ifdef __AVX__
#define RA_HAVE_AVX 1
#else
#define RA_HAVE_AVX 0
#endif
#endif

#ifndef RA_HAVE_AVX2
#ifdef __AVX2__
#define RA_HAVE_AVX2 1
#else
#define RA_HAVE_AVX2 0
#endif
#endif

#endif // RA_ENGINE_HAVE_INTRINSICS_H
