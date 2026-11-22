/*
 * Replaces <assert.h> for the zmap sources. Upstream asserts on input from
 * the options (for example, too many entries in a list). abort() would end
 * the Node.js process, so a failed assertion fails the scan instead.
 *
 * <assert.h> has no include guard by design: each inclusion defines assert again.
 */
#ifndef LIBZMAP_ASSERT_DECL
#define LIBZMAP_ASSERT_DECL
void libzmap_assert_fail(const char *expr, const char *file, int line)
    __attribute__((noreturn));
#endif

#undef assert
#ifdef NDEBUG
#define assert(e) ((void)0)
#else
#define assert(e) ((e) ? (void)0 : libzmap_assert_fail(#e, __FILE__, __LINE__))
#endif

#if !defined(__cplusplus) && !defined(static_assert)
#define static_assert _Static_assert
#endif
