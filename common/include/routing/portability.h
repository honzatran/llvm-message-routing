


#ifndef ROUTING_PORTABILITY_H
#define ROUTING_PORTABILITY_H

#define RA_ATTR_WEAK __attribute__((__weak__))

/// instruct the compiler not to inline function, usefull when we want 
/// to remove a function call from a hot path
#define RA_DONT_INLINE __attribute__((__noinline__))

/**
 * This is copied from folly/likely.h
 */

#if __GNUC__
#define RA_DETAIL_BUILTIN_EXPECT(b, t) (__builtin_expect(b, t))
#else
#define RA_DETAIL_BUILTIN_EXPECT(b, t) b
#endif

///  Likeliness annotations
//  //  Useful when the author has better knowledge than the compiler of whether
//  //  the branch condition is overwhelmingly likely to take a specific value.
//  //
//  //  Useful when the author has better knowledge than the compiler of which code
//  //  paths are designed as the fast path and which are designed as the slow path,
//  //  and to force the compiler to optimize for the fast path, even when it is not
//  //  overwhelmingly likely.
//

#define RA_LIKELY(x) RA_DETAIL_BUILTIN_EXPECT((x), 1)
#define RA_UNLIKELY(x) RA_DETAIL_BUILTIN_EXPECT((x), 0)

#endif
