/* Includes upstream zmap.c to run its main() in a thread. The macros below adapt it to a library. */
#include "shim/libzmap-shim.h"
#include "libzmap.h"

#include <pthread.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>

static int lz_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                             void *(*start)(void *), void *arg);
static int lz_pthread_join(pthread_t thread, void **value);
static int lz_fclose(FILE *fp);
static int lz_drop_privs(void);

#define main zmap_cli_main
/* Upstream changes to user "nobody" after it opens the sockets. That would change the Node.js process. */
#define drop_privs lz_drop_privs
/* Records that scan threads run, so exit() can no longer unwind. */
#define pthread_create lz_pthread_create
#define pthread_join lz_pthread_join
/* Upstream closes its log stream at the end, which is stderr by default. */
#define fclose lz_fclose

#include "zmap.c"

#undef main
#undef drop_privs
#undef pthread_create
#undef pthread_join
#undef fclose

static struct state_conf lz_initial_zconf;
static struct state_send lz_initial_zsend;
static struct state_recv lz_initial_zrecv;
static int lz_saved_initial;

/* The scan threads that upstream started and has not joined yet. */
#define LZ_MAX_THREADS 512
static pthread_t lz_threads[LZ_MAX_THREADS];
static int lz_thread_open[LZ_MAX_THREADS];
static int lz_thread_count;

static int
lz_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                  void *(*start)(void *), void *arg)
{
    int rc;

    lz_threads_started = 1;
    rc = pthread_create(thread, attr, start, arg);
    if (rc == 0 && lz_thread_count < LZ_MAX_THREADS) {
        lz_threads[lz_thread_count] = *thread;
        lz_thread_open[lz_thread_count] = 1;
        lz_thread_count++;
    }
    return rc;
}

static int
lz_pthread_join(pthread_t thread, void **value)
{
    int i;

    for (i = 0; i < lz_thread_count; i++) {
        if (lz_thread_open[i] && pthread_equal(lz_threads[i], thread))
            lz_thread_open[i] = 0;
    }
    return pthread_join(thread, value);
}

/*
 * The runner failed while scan threads run. Stops them and joins those that
 * upstream did not join, so the runner can unwind. Returns 0 when all ended.
 */
int
lz_join_threads(void)
{
    int i, failed = 0;

    lz_abort_threads();
    for (i = 0; i < lz_thread_count; i++) {
        if (!lz_thread_open[i])
            continue;
        lz_thread_open[i] = 0;
        if (pthread_join(lz_threads[i], NULL) != 0)
            failed = 1;
    }
    lz_thread_count = 0;
    return failed ? -1 : 0;
}

static int
lz_fclose(FILE *fp)
{
    if (fp == NULL || fp == stdin || fp == stdout || fp == stderr)
        return 0;
    return fclose(fp);
}

static int
lz_drop_privs(void)
{
    return EXIT_SUCCESS;
}

/* zmap keeps its settings and counters in globals. Each scan starts from the upstream defaults. */
void
lz_reset_globals(void)
{
    if (!lz_saved_initial) {
        lz_initial_zconf = zconf;
        lz_initial_zsend = zsend;
        lz_initial_zrecv = zrecv;
        lz_saved_initial = 1;
    }
    zconf = lz_initial_zconf;
    zsend = lz_initial_zsend;
    zrecv = lz_initial_zrecv;

    lz_threads_started = 0;
    lz_thread_count = 0;
    lz_thread_failed = 0;
    lz_exit_code = 0;
}

/* Makes every scan thread finish: senders stop, the receiver skips the cooldown. */
void
lz_abort_threads(void)
{
    zconf.max_runtime = 1;
    zconf.cooldown_secs = 0;
    if (!zsend.complete) {
        zsend.finish = now();
        zsend.complete = 1;
    }
    zrecv.complete = 1;
    /* The runner waits for this before it starts the senders. */
    pthread_mutex_lock(&recv_ready_mutex);
    zconf.recv_ready = 1;
    pthread_mutex_unlock(&recv_ready_mutex);
}

/* First call: stop sending and wait for the cooldown. force: stop now. */
void
lz_request_stop(int force)
{
    if (force || zconf.max_runtime == 1) {
        lz_abort_threads();
        return;
    }
    /* The send loop compares max_runtime with the time since the start. */
    zconf.max_runtime = 1;
}

static void
lz_fail(struct lz_run_result *result, const char *message)
{
    result->failed = 1;
    result->exit_code = 1;
    snprintf(result->message, sizeof(result->message), "%s", message);
}

/* Upstream main() with the given arguments. Returns 0, or -1 with result->failed set. */
int
lz_run(int argc, char **argv, struct lz_run_result *result)
{
    jmp_buf trap;
    int rc;

    memset(result, 0, sizeof(*result));
    lz_runner_thread = pthread_self();

    if (setjmp(trap)) {
        /* zmap called exit() or log_fatal(). */
        lz_exit_jmp = NULL;
        result->failed = 1;
        result->exit_code = lz_exit_code ? lz_exit_code : 1;
        return -1;
    }
    lz_exit_jmp = &trap;

    /* getopt keeps its position in globals. gengetopt resets optind to 0, which also resets BSD getopt. */
    optind = 0;
    rc = zmap_cli_main(argc, argv);
    lz_exit_jmp = NULL;

    if (lz_thread_failed) {
        /* A scan thread called exit() and ended, so the results are incomplete. */
        result->failed = 1;
        result->exit_code = lz_exit_code ? lz_exit_code : 1;
        return -1;
    }
    if (rc != 0) {
        lz_fail(result, "zmap stopped with an error");
        result->exit_code = rc;
        return -1;
    }
    return 0;
}
