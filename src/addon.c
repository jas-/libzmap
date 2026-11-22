/* N-API binding: start(argv, verbosity, onEvents), stop(force), isRunning(). One scan per process. */
#define NAPI_VERSION 8
#include <node_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libzmap.h"

/* output.c */
extern char *volatile lz_scan_meta;

static volatile int lz_busy = 0;

static const char *level_names[] = {
    "fatal", "error", "warn", "info", "debug", "trace"
};

#define NAPI_CALL(env, call)                                            \
    do {                                                                \
        if ((call) != napi_ok) {                                        \
            const napi_extended_error_info *info = NULL;                \
            napi_get_last_error_info((env), &info);                     \
            napi_throw_error((env), NULL,                               \
                info && info->error_message ? info->error_message       \
                                            : "N-API call failed");     \
            return NULL;                                                \
        }                                                               \
    } while (0)

static void
event_free(struct lz_event *ev)
{
    free(ev->text);
    free(ev->meta);
}

static void
scan_free(struct lz_scan *scan)
{
    size_t i;
    int j;

    for (i = 0; i < scan->count; i++)
        event_free(&scan->events[i]);
    free(scan->events);
    if (scan->has_done)
        event_free(&scan->done);
    for (j = 0; j < scan->argc; j++)
        free(scan->argv[j]);
    free(scan->argv);
    pthread_mutex_destroy(&scan->lock);
    free(scan);
}

void
lz_notify(struct lz_scan *scan)
{
    napi_call_threadsafe_function((napi_threadsafe_function)scan->tsfn,
                                  NULL, napi_tsfn_nonblocking);
}

static char *
error_text(struct lz_scan *scan, const struct lz_run_result *result)
{
    char buffer[LZ_ERROR_LINES * 514 + 300];
    size_t used = 0;
    unsigned first, i;

    pthread_mutex_lock(&scan->lock);
    first = scan->error_line_count > LZ_ERROR_LINES
                ? scan->error_line_count - LZ_ERROR_LINES : 0;
    for (i = first; i < scan->error_line_count; i++) {
        int n = snprintf(buffer + used, sizeof(buffer) - used, "%s%s",
                         used ? "\n" : "",
                         scan->error_lines[i % LZ_ERROR_LINES]);
        if (n < 0 || (size_t)n >= sizeof(buffer) - used)
            break;
        used += (size_t)n;
    }
    pthread_mutex_unlock(&scan->lock);

    if (used == 0) {
        if (result->message[0])
            snprintf(buffer, sizeof(buffer), "%s", result->message);
        else
            snprintf(buffer, sizeof(buffer),
                     "zmap stopped with exit code %d", result->exit_code);
    }
    return strdup(buffer);
}

void
lz_complete(struct lz_scan *scan, struct lz_run_result *result)
{
    struct lz_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = LZ_EV_DONE;
    ev.failed = result->failed;
    ev.exit_code = result->exit_code;
    ev.meta = result->meta;
    result->meta = NULL;
    if (result->failed) {
        ev.text = error_text(scan, result);
        ev.text_len = ev.text ? strlen(ev.text) : 0;
    }

    /* Clear busy before queueing done, so a 'done' handler can start the next scan. */
    lz_current = NULL;
    lz_busy = 0;
    pthread_mutex_lock(&scan->lock);
    scan->done = ev;
    scan->has_done = 1;
    if (!scan->notify_pending) {
        scan->notify_pending = 1;
        lz_notify(scan);
    }
    pthread_mutex_unlock(&scan->lock);
    napi_release_threadsafe_function((napi_threadsafe_function)scan->tsfn,
                                     napi_tsfn_release);
}

static void *
scan_thread(void *arg)
{
    struct lz_scan *scan = arg;
    struct lz_run_result result;

    lz_run(scan->argc, scan->argv, &result);
    result.meta = lz_scan_meta;
    lz_scan_meta = NULL;
    lz_complete(scan, &result);
    return NULL;
}

static napi_status
set_string(napi_env env, napi_value obj, const char *key, const char *value)
{
    napi_value v;
    napi_status s = napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &v);
    return s == napi_ok ? napi_set_named_property(env, obj, key, v) : s;
}

static napi_status
set_number(napi_env env, napi_value obj, const char *key, double value)
{
    napi_value v;
    napi_status s = napi_create_double(env, value, &v);
    return s == napi_ok ? napi_set_named_property(env, obj, key, v) : s;
}

static napi_status
set_null(napi_env env, napi_value obj, const char *key)
{
    napi_value v;
    napi_get_null(env, &v);
    return napi_set_named_property(env, obj, key, v);
}

static void
set_number_or_null(napi_env env, napi_value obj, const char *key, double value)
{
    if (value < 0)
        set_null(env, obj, key);
    else
        set_number(env, obj, key, value);
}

static napi_value
event_to_js(napi_env env, const struct lz_event *ev)
{
    napi_value obj, v;
    const struct lz_progress *p = &ev->progress;

    if (napi_create_object(env, &obj) != napi_ok)
        return NULL;

    switch (ev->type) {
    case LZ_EV_RESULT:
        set_string(env, obj, "type", "result");
        napi_create_string_utf8(env, ev->text ? ev->text : "{}",
                                ev->text_len, &v);
        napi_set_named_property(env, obj, "json", v);
        break;

    case LZ_EV_PROGRESS:
        set_string(env, obj, "type", "progress");
        set_number_or_null(env, obj, "percent", p->percent);
        set_number(env, obj, "elapsed", p->elapsed);
        set_number_or_null(env, obj, "remaining", p->remaining);
        set_number(env, obj, "sent", (double)p->sent);
        set_number(env, obj, "sendRate", p->send_rate);
        set_number(env, obj, "sendRateAvg", p->send_rate_avg);
        set_number(env, obj, "successUnique", (double)p->success_unique);
        set_number(env, obj, "received", (double)p->received);
        set_number(env, obj, "recvRate", p->recv_rate);
        set_number(env, obj, "recvRateAvg", p->recv_rate_avg);
        set_number(env, obj, "hitrate", p->hitrate);
        if (p->app_hitrate >= 0) {
            set_number(env, obj, "appSuccessUnique",
                       (double)p->app_success_unique);
            set_number(env, obj, "appHitrate", p->app_hitrate);
        }
        set_number(env, obj, "drops", (double)p->drops);
        set_number(env, obj, "sendFailures", (double)p->send_failures);
        napi_get_boolean(env, p->complete != 0, &v);
        napi_set_named_property(env, obj, "sendComplete", v);
        break;

    case LZ_EV_LOG:
        set_string(env, obj, "type", "log");
        set_string(env, obj, "level",
                   ev->level >= 0 && ev->level <= 5 ? level_names[ev->level]
                                                    : "info");
        set_string(env, obj, "module", ev->module);
        set_string(env, obj, "message", ev->text ? ev->text : "");
        break;

    case LZ_EV_DONE:
        set_string(env, obj, "type", "done");
        if (ev->failed) {
            set_string(env, obj, "error", ev->text ? ev->text : "scan failed");
            set_number(env, obj, "exitCode", ev->exit_code);
        } else {
            set_null(env, obj, "error");
        }
        if (ev->meta)
            set_string(env, obj, "metadata", ev->meta);
        else
            set_null(env, obj, "metadata");
        break;
    }
    return obj;
}

static void
call_js(napi_env env, napi_value js_callback, void *context, void *data)
{
    struct lz_scan *scan = context;
    struct lz_event *events;
    struct lz_event done;
    size_t count, i;
    int has_done;
    napi_value array, undefined;
    (void)data;

    pthread_mutex_lock(&scan->lock);
    events = scan->events;
    count = scan->count;
    scan->events = NULL;
    scan->count = 0;
    scan->capacity = 0;
    scan->notify_pending = 0;
    done = scan->done;
    has_done = scan->has_done;
    scan->has_done = 0;
    pthread_mutex_unlock(&scan->lock);

    if (env != NULL && js_callback != NULL && count + has_done > 0
        && napi_create_array_with_length(env, count + has_done, &array) == napi_ok) {
        for (i = 0; i < count; i++) {
            napi_value obj = event_to_js(env, &events[i]);
            if (obj)
                napi_set_element(env, array, (uint32_t)i, obj);
        }
        if (has_done) {
            napi_value obj = event_to_js(env, &done);
            if (obj)
                napi_set_element(env, array, (uint32_t)count, obj);
        }
        napi_get_undefined(env, &undefined);
        napi_call_function(env, undefined, js_callback, 1, &array, NULL);
    }

    for (i = 0; i < count; i++)
        event_free(&events[i]);
    free(events);
    if (has_done)
        event_free(&done);
}

static void
tsfn_finalize(napi_env env, void *finalize_data, void *hint)
{
    (void)env;
    (void)hint;
    scan_free(finalize_data);
}

static napi_value
Start(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3], resource_name;
    bool is_array;
    uint32_t length, i;
    int32_t verbosity;
    napi_valuetype type, vtype;
    struct lz_scan *scan;
    pthread_attr_t attr;
    pthread_t thread;
    napi_threadsafe_function tsfn;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
    if (argc < 3) {
        napi_throw_type_error(env, NULL, "start(argv, verbosity, onEvents) needs 3 arguments");
        return NULL;
    }
    NAPI_CALL(env, napi_is_array(env, args[0], &is_array));
    NAPI_CALL(env, napi_typeof(env, args[1], &vtype));
    NAPI_CALL(env, napi_typeof(env, args[2], &type));
    if (!is_array || vtype != napi_number || type != napi_function) {
        napi_throw_type_error(env, NULL,
            "start(argv, verbosity, onEvents) needs an array, a number and a function");
        return NULL;
    }
    NAPI_CALL(env, napi_get_array_length(env, args[0], &length));
    NAPI_CALL(env, napi_get_value_int32(env, args[1], &verbosity));
    if (lz_poisoned) {
        napi_throw_error(env, "ERR_ZMAP_POISONED",
            "An earlier scan failed fatally. Restart the process before the next scan.");
        return NULL;
    }
    if (lz_busy) {
        napi_throw_error(env, "ERR_ZMAP_BUSY",
            "A scan is already running. zmap keeps global state, so a "
            "process runs one scan at a time.");
        return NULL;
    }

    scan = calloc(1, sizeof(*scan));
    if (scan == NULL) {
        napi_throw_error(env, NULL, "out of memory");
        return NULL;
    }
    pthread_mutex_init(&scan->lock, NULL);

    scan->verbosity = verbosity;
    /* argv[0], the arguments, and the NULL that ends argv. */
    scan->argv = calloc((size_t)length + 2, sizeof(char *));
    if (scan->argv == NULL || (scan->argv[0] = strdup("zmap")) == NULL) {
        scan_free(scan);
        napi_throw_error(env, NULL, "out of memory");
        return NULL;
    }
    scan->argc = 1;
    for (i = 0; i < length; i++) {
        napi_value item;
        size_t size;

        if (napi_get_element(env, args[0], i, &item) != napi_ok
            || napi_typeof(env, item, &type) != napi_ok
            || type != napi_string) {
            scan_free(scan);
            napi_throw_type_error(env, NULL, "every argument must be a string");
            return NULL;
        }
        napi_get_value_string_utf8(env, item, NULL, 0, &size);
        scan->argv[scan->argc] = malloc(size + 1);
        if (scan->argv[scan->argc] == NULL) {
            scan_free(scan);
            napi_throw_error(env, NULL, "out of memory");
            return NULL;
        }
        napi_get_value_string_utf8(env, item, scan->argv[scan->argc],
                                   size + 1, &size);
        scan->argc++;
    }

    if (napi_create_string_utf8(env, "zmap", NAPI_AUTO_LENGTH,
                                &resource_name) != napi_ok
        || napi_create_threadsafe_function(env, args[2], NULL, resource_name,
            0, 1, scan, tsfn_finalize, scan, call_js, &tsfn) != napi_ok) {
        scan_free(scan);
        napi_throw_error(env, NULL, "cannot create the event channel");
        return NULL;
    }
    scan->tsfn = tsfn;

    lz_reset_globals();
    lz_set_verbosity(scan->verbosity);
    lz_busy = 1;
    lz_current = scan;

    /* zmap.c main() has large locals. The macOS default thread stack is 512 KB. */
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 16 * 1024 * 1024);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&thread, &attr, scan_thread, scan) != 0) {
        pthread_attr_destroy(&attr);
        lz_current = NULL;
        lz_busy = 0;
        /* Releasing the last reference calls tsfn_finalize. */
        napi_release_threadsafe_function(tsfn, napi_tsfn_abort);
        napi_throw_error(env, NULL, "cannot start the scan thread");
        return NULL;
    }
    pthread_attr_destroy(&attr);
    return NULL;
}

static napi_value
Stop(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value arg;
    bool force = false;
    napi_valuetype type;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &arg, NULL, NULL));
    if (argc >= 1 && napi_typeof(env, arg, &type) == napi_ok
        && type == napi_boolean)
        napi_get_value_bool(env, arg, &force);

    if (lz_busy)
        lz_request_stop(force ? 1 : 0);
    return NULL;
}

static napi_value
IsRunning(napi_env env, napi_callback_info info)
{
    napi_value result;
    (void)info;
    napi_get_boolean(env, lz_busy != 0, &result);
    return result;
}

static napi_value
Init(napi_env env, napi_value exports)
{
    napi_property_descriptor props[] = {
        { "start", NULL, Start, NULL, NULL, NULL, napi_default, NULL },
        { "stop", NULL, Stop, NULL, NULL, NULL, napi_default, NULL },
        { "isRunning", NULL, IsRunning, NULL, NULL, NULL, napi_default, NULL },
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                          sizeof(props) / sizeof(props[0]),
                                          props));
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
