/* Force-included into upstream sources: routes exit(), printf(), stderr writes and signal() to hooks.c. */
#ifndef LIBZMAP_SHIM_H
#define LIBZMAP_SHIM_H

#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

void libzmap_exit(int code) __attribute__((noreturn));
int libzmap_fprintf(FILE *fp, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
int libzmap_vfprintf(FILE *fp, const char *fmt, va_list ap);
int libzmap_printf(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
int libzmap_signal(int sig);

#undef exit
#define exit(code) libzmap_exit(code)

#undef fprintf
#define fprintf(...) libzmap_fprintf(__VA_ARGS__)

#undef printf
#define printf(...) libzmap_printf(__VA_ARGS__)

#undef vfprintf
#define vfprintf(fp, fmt, ap) libzmap_vfprintf((fp), (fmt), (ap))

/* zmap sets SIGUSR1/SIGUSR2 handlers to change the send rate. Node.js uses SIGUSR1. */
#undef signal
#define signal(sig, handler) ((void)(handler), libzmap_signal(sig))

#endif
