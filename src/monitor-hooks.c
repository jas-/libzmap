/* Upstream monitor.c, with its status line on stderr replaced by progress events. */
#include "shim/libzmap-shim.h"
#include "libzmap.h"

#define monitor_run lz_orig_monitor_run
#include "monitor.c"
#undef monitor_run

static void
lz_progress(export_status_t *exp)
{
    struct lz_event ev;
    struct lz_progress *p = &ev.progress;

    memset(&ev, 0, sizeof(ev));
    ev.type = LZ_EV_PROGRESS;
    p->percent = isfinite(exp->percent_complete) ? exp->percent_complete : -1;
    if (p->percent > 100.0)
        p->percent = 100.0;
    p->elapsed = exp->time_past;
    p->remaining = isfinite((double)exp->time_remaining)
                       ? (double)exp->time_remaining : -1;
    p->sent = exp->total_sent;
    p->tried = exp->total_tried_sent;
    p->targets = zsend.targets_scanned;
    p->send_rate = exp->complete ? 0 : exp->send_rate;
    p->send_rate_avg = exp->send_rate_avg;
    p->success_unique = exp->recv_success_unique;
    p->app_success_unique = exp->app_recv_success_unique;
    p->received = exp->total_recv;
    p->recv_rate = exp->recv_rate;
    p->recv_rate_avg = exp->recv_avg;
    p->hitrate = exp->hitrate;
    p->app_hitrate = zconf.fsconf.app_success_index >= 0 ? exp->app_hitrate : -1;
    p->drops = exp->pcap_drop_total;
    p->send_failures = exp->fail_total;
    p->complete = exp->complete ? 1 : 0;
    lz_push(lz_current, &ev);
}

static void
lz_export_then_update(int_status_t *internal, iterator_t *it,
                      export_status_t *exp, pthread_mutex_t *lock)
{
    update_pcap_stats(lock);
    export_stats(internal, exp, it);
    log_drop_warnings(exp);
    check_min_hitrate(exp);
    check_max_sendto_failures(exp);
    if (!zconf.quiet)
        lz_progress(exp);
    if (status_fd)
        update_status_updates_file(exp, status_fd);
}

void
monitor_run(iterator_t *it, pthread_mutex_t *lock)
{
    int_status_t *internal = xcalloc(1, sizeof(int_status_t));
    export_status_t *exp = xcalloc(1, sizeof(export_status_t));

    internal->last_now = zsend.start;
    while (!(zsend.complete && zrecv.complete)) {
        lz_export_then_update(internal, it, exp, lock);
        /* Short sleeps, so a stopped scan ends quickly. */
        for (int i = 0; i < 10 * UPDATE_INTERVAL
                        && !(zsend.complete && zrecv.complete); i++)
            usleep(100000);
    }
    lz_export_then_update(internal, it, exp, lock);

    if (status_fd) {
        fflush(status_fd);
        fclose(status_fd);
        /* Upstream leaves the pointer set. The next scan must not write to the closed file. */
        status_fd = NULL;
    }
    free(internal);
    free(exp);
}
