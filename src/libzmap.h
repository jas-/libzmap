/* Internal interface shared by addon.c, hooks.c, runner.c, monitor.c and output.c. */
#ifndef LIBZMAP_H
#define LIBZMAP_H

#include <pthread.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

enum lz_event_type {
    LZ_EV_RESULT,
    LZ_EV_PROGRESS,
    LZ_EV_LOG,
    LZ_EV_DONE
};

struct lz_progress {
    double percent;          /* -1 when unknown */
    double elapsed;          /* seconds */
    double remaining;        /* seconds, -1 when unknown */
    uint64_t sent;
    uint64_t tried;
    uint64_t targets;        /* targets scanned */
    double send_rate;
    double send_rate_avg;
    uint64_t success_unique;
    uint64_t app_success_unique;
    uint64_t received;       /* every validated response */
    double recv_rate;
    double recv_rate_avg;
    double hitrate;
    double app_hitrate;      /* -1 when the probe module has no app success */
    uint64_t drops;
    uint64_t send_failures;
    int complete;            /* sending is done, cooldown runs */
};

struct lz_event {
    enum lz_event_type type;

    /* LZ_EV_PROGRESS */
    struct lz_progress progress;

    /* LZ_EV_LOG */
    int level;               /* 0 fatal, 1 error, 2 warn, 3 info, 4 debug, 5 trace */
    char module[32];

    /* LZ_EV_DONE */
    int failed;
    int exit_code;

    /*
     * Owned text. LZ_EV_RESULT: one JSON object. LZ_EV_LOG: the message.
     * LZ_EV_DONE: the error message, or NULL.
     */
    char *text;
    size_t text_len;

    /* LZ_EV_DONE: zmap's scan metadata as JSON, or NULL. */
    char *meta;
};

#define LZ_ERROR_LINES 8

struct lz_scan {
    pthread_mutex_t lock;
    struct lz_event *events;
    size_t count;
    size_t capacity;
    int notify_pending;

    /* Kept out of the queue, so an allocation failure cannot drop it. */
    struct lz_event done;
    int has_done;

    void *tsfn;              /* napi_threadsafe_function */

    /* zmap arguments, argv[0] included. */
    int argc;
    char **argv;
    int verbosity;

    /* The last error and fatal log lines. They become the error message. */
    char error_lines[LZ_ERROR_LINES][512];
    unsigned error_line_count;
};

struct lz_run_result {
    int failed;
    int exit_code;           /* set when zmap called exit() */
    char message[256];       /* set by the runner for its own errors */
    char *meta;              /* scan metadata JSON (malloc), or NULL */
};

/* The scan that receives events. NULL when no scan runs. */
extern struct lz_scan *volatile lz_current;

/* Exit trap state (hooks.c, used by runner.c). */
extern pthread_t lz_runner_thread;
extern jmp_buf *volatile lz_exit_jmp;
extern volatile int lz_exit_code;
extern volatile int lz_threads_started;
extern volatile int lz_thread_failed;
extern volatile int lz_poisoned;

/* hooks.c */
void lz_push(struct lz_scan *scan, struct lz_event *ev);
void lz_log_text(int level, const char *module, const char *text);
void lz_set_verbosity(int level);

/* runner.c */
int lz_run(int argc, char **argv, struct lz_run_result *result);
void lz_reset_globals(void);
void lz_request_stop(int force);
void lz_abort_threads(void);
int lz_join_threads(void);

/* addon.c: finishes a scan and hands the done event to JavaScript. */
void lz_complete(struct lz_scan *scan, struct lz_run_result *result);
void lz_notify(struct lz_scan *scan);

#endif
