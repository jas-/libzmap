/* Replaces the upstream logger (lib/logger.c), exit() and stderr output with events. */
#include "libzmap.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "logger.h"

/* Declared in shim/libzmap-shim.h and shim/include/assert.h, which this file does not include. */
void libzmap_exit(int code) __attribute__((noreturn));
void libzmap_assert_fail(const char *expr, const char *file, int line)
    __attribute__((noreturn));

struct lz_scan *volatile lz_current = NULL;

pthread_t lz_runner_thread;
jmp_buf *volatile lz_exit_jmp = NULL;
volatile int lz_exit_code = 0;
volatile int lz_threads_started = 0;
volatile int lz_thread_failed = 0;
volatile int lz_poisoned = 0;

static volatile int lz_verbosity = ZLOG_INFO;

void
lz_push(struct lz_scan *scan, struct lz_event *ev)
{
    if (scan == NULL) {
        free(ev->text);
        free(ev->meta);
        return;
    }

    pthread_mutex_lock(&scan->lock);
    if (scan->count == scan->capacity) {
        size_t capacity = scan->capacity ? scan->capacity * 2 : 64;
        struct lz_event *events = realloc(scan->events,
                                          capacity * sizeof(*events));
        if (events == NULL) {
            /* Drop the event. Do not stop the scan for a lost event. */
            pthread_mutex_unlock(&scan->lock);
            free(ev->text);
            free(ev->meta);
            return;
        }
        scan->events = events;
        scan->capacity = capacity;
    }
    scan->events[scan->count++] = *ev;

    /* One wake-up per batch, not per event. */
    if (!scan->notify_pending) {
        scan->notify_pending = 1;
        lz_notify(scan);
    }
    pthread_mutex_unlock(&scan->lock);
}

static char *
lz_strndup(const char *text, size_t length)
{
    char *copy = malloc(length + 1);
    if (copy) {
        memcpy(copy, text, length);
        copy[length] = '\0';
    }
    return copy;
}

static void
lz_log_line(int level, const char *module, const char *line, size_t length)
{
    struct lz_scan *scan = lz_current;
    struct lz_event ev;

    while (length && (line[length-1] == ' ' || line[length-1] == '\r'
                      || line[length-1] == '\t'))
        length--;
    if (length == 0 || scan == NULL)
        return;

    if (level <= ZLOG_ERROR) {
        pthread_mutex_lock(&scan->lock);
        snprintf(scan->error_lines[scan->error_line_count % LZ_ERROR_LINES],
                 sizeof(scan->error_lines[0]), "%s%s%.*s",
                 module ? module : "", module ? ": " : "",
                 (int)length, line);
        scan->error_line_count++;
        pthread_mutex_unlock(&scan->lock);
    }

    if (level > lz_verbosity)
        return;

    memset(&ev, 0, sizeof(ev));
    ev.type = LZ_EV_LOG;
    ev.level = level;
    snprintf(ev.module, sizeof(ev.module), "%s", module ? module : "");
    ev.text = lz_strndup(line, length);
    ev.text_len = length;
    lz_push(scan, &ev);
}

void
lz_log_text(int level, const char *module, const char *text)
{
    while (*text) {
        const char *end = strchr(text, '\n');
        size_t length = end ? (size_t)(end - text) : strlen(text);

        lz_log_line(level, module, text, length);
        text += length;
        if (*text == '\n')
            text++;
    }
}

static void
lz_vlog(int level, const char *module, const char *fmt, va_list ap)
{
    char buffer[4096];

    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    lz_log_text(level, module, buffer);
}

void
lz_set_verbosity(int level)
{
    lz_verbosity = level;
}

/* ---- lib/logger.c ---- */

int
log_init(FILE *stream, enum LogLevel level, int syslog_enabled,
         const char *appname)
{
    (void)stream;
    (void)syslog_enabled;
    (void)appname;
    /* The verbosity option sets the level. lz_set_verbosity() has it already. */
    (void)level;
    return 0;
}

#define LZ_LOG_FN(name, level)                                          \
    int name(const char *module, const char *fmt, ...)                  \
    {                                                                   \
        va_list ap;                                                     \
        va_start(ap, fmt);                                              \
        lz_vlog((level), module, fmt, ap);                              \
        va_end(ap);                                                     \
        return EXIT_SUCCESS;                                            \
    }

LZ_LOG_FN(log_error, ZLOG_ERROR)
LZ_LOG_FN(log_warn, ZLOG_WARN)
LZ_LOG_FN(log_info, ZLOG_INFO)
LZ_LOG_FN(log_debug, ZLOG_DEBUG)

#ifdef DEBUG
LZ_LOG_FN(log_trace, ZLOG_TRACE)
#endif

int
log_fatal(const char *module, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    lz_vlog(ZLOG_FATAL, module, fmt, ap);
    va_end(ap);
    libzmap_exit(EXIT_FAILURE);
}

void
check_and_log_file_error(FILE *file, const char *name)
{
    if (ferror(file))
        log_fatal(name, "unable to write to file");
}

size_t
dstrftime(char *buf, size_t maxsize, const char *format, double tm)
{
    time_t sec = (time_t)floor(tm);
    struct tm local;

    localtime_r(&sec, &local);
    return strftime(buf, maxsize, format, &local);
}

/* ---- stdio and exit() from upstream sources (see shim/libzmap-shim.h) ---- */

int
libzmap_vfprintf(FILE *fp, const char *fmt, va_list ap)
{
    char buffer[4096];
    int length;

    if (fp != stderr && fp != stdout)
        return vfprintf(fp, fmt, ap);

    /* Upstream writes to stdout only for --help and the lists, which the module does not offer. */
    length = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    /* stdout: the probe dump of a dry run. */
    lz_log_text(fp == stderr ? ZLOG_ERROR : ZLOG_DEBUG, NULL, buffer);
    return length;
}

int
libzmap_fprintf(FILE *fp, const char *fmt, ...)
{
    va_list ap;
    int length;

    va_start(ap, fmt);
    length = libzmap_vfprintf(fp, fmt, ap);
    va_end(ap);
    return length;
}

int
libzmap_printf(const char *fmt, ...)
{
    va_list ap;
    int length;

    va_start(ap, fmt);
    length = libzmap_vfprintf(stdout, fmt, ap);
    va_end(ap);
    return length;
}

void
libzmap_assert_fail(const char *expr, const char *file, int line)
{
    const char *base = strrchr(file, '/');

    log_fatal("zmap", "internal check failed: %s (%s:%d)", expr,
              base ? base + 1 : file, line);
}

/* ---- Files that upstream opens (see shim/libzmap-shim.h) ---- */

#define LZ_MAX_FILES 64
static FILE *lz_files[LZ_MAX_FILES];
static pthread_mutex_t lz_files_lock = PTHREAD_MUTEX_INITIALIZER;

FILE *
libzmap_fopen(const char *path, const char *mode)
{
    FILE *fp = fopen(path, mode);
    int i;

    if (fp == NULL)
        return NULL;
    pthread_mutex_lock(&lz_files_lock);
    for (i = 0; i < LZ_MAX_FILES; i++) {
        if (lz_files[i] == NULL) {
            lz_files[i] = fp;
            break;
        }
    }
    pthread_mutex_unlock(&lz_files_lock);
    return fp;
}

int
libzmap_fclose(FILE *fp)
{
    int i;

    if (fp == NULL || fp == stdin || fp == stdout || fp == stderr)
        return 0;
    pthread_mutex_lock(&lz_files_lock);
    for (i = 0; i < LZ_MAX_FILES; i++) {
        if (lz_files[i] == fp)
            lz_files[i] = NULL;
    }
    pthread_mutex_unlock(&lz_files_lock);
    return fclose(fp);
}

/* Closes the files that upstream left open: on a failure, and the metadata file, which upstream never closes. */
void
lz_close_files(void)
{
    int i;

    pthread_mutex_lock(&lz_files_lock);
    for (i = 0; i < LZ_MAX_FILES; i++) {
        if (lz_files[i] != NULL) {
            fclose(lz_files[i]);
            lz_files[i] = NULL;
        }
    }
    pthread_mutex_unlock(&lz_files_lock);
}

/*
 * ---- lib/lockfd.c ----
 * Upstream locks stdout and stderr around its terminal output, and can call
 * exit() with the lock held. That would leave the lock with an ended thread.
 * The module sends that output to events, which hooks.c serializes, so no lock is needed.
 */
int
lock_file(FILE *f)
{
    (void)f;
    return 0;
}

int
unlock_file(FILE *f)
{
    (void)f;
    return 0;
}

int
libzmap_signal(int sig)
{
    (void)sig;
    return 0;
}

void
libzmap_exit(int code)
{
    jmp_buf *target = lz_exit_jmp;
    int on_runner = target != NULL
                    && pthread_equal(pthread_self(), lz_runner_thread);

    /* Configuration, before any scan thread: unwind to lz_run(). */
    if (on_runner && !lz_threads_started) {
        lz_exit_code = code;
        lz_exit_jmp = NULL;
        longjmp(*target, 1);
    }

    if (target != NULL && !on_runner) {
        /*
         * A scan thread (send, receive or monitor). Tell the other threads to
         * stop, and end only this thread. lz_run() then reports the failure.
         */
        lz_exit_code = code;
        lz_thread_failed = 1;
        lz_abort_threads();
        pthread_exit(NULL);
    }

    if (on_runner) {
        /*
         * The runner, while scan threads run. Stop and join the threads,
         * then unwind. If a thread cannot be joined, refuse later scans.
         */
        lz_exit_code = code;
        lz_thread_failed = 1;
        lz_exit_jmp = NULL;
        if (lz_join_threads() != 0)
            lz_poisoned = 1;
        longjmp(*target, 1);
    }

    /* No scan runs. Never end the Node.js process. */
    fprintf(stderr, "libzmap: zmap called exit(%d) outside a scan. "
                    "Later scans will be refused.\n", code);
    lz_poisoned = 1;
    pthread_exit(NULL);
}
