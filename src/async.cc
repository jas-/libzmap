#include <node.h>
#include <v8.h>

#include "./libzmap.h"

extern "C" {
#include <unistd.h>
#include <pcap/pcap.h>
#include <pthread.h>

#include "zmap-1.2.1/lib/xalloc.h"
#include "zmap-1.2.1/lib/logger.h"

#include "zmap-1.2.1/src/state.h"
#include "zmap-1.2.1/src/iterator.h"
#include "zmap-1.2.1/src/recv.h"
#include "zmap-1.2.1/src/send.h"

#include "zmap-1.2.1/src/probe_modules/probe_modules.h"
#include "zmap-1.2.1/src/output_modules/output_modules.h"
}

using namespace node;
using namespace v8;

pthread_mutex_t recv_ready_mutex = PTHREAD_MUTEX_INITIALIZER;

void libzmap::Threads(void) {
	libzmap lz;
	iterator_t *it = send_init();

	if (zconf.output_module && zconf.output_module->init) {
		zconf.output_module->init(&zconf, zconf.output_fields,
				zconf.output_fields_len);
	}

	if (!it) {
		ThrowException(Exception::TypeError(String::New("Unable to initialize sending component")));
	}

	if (zconf.output_module && zconf.output_module->start) {
		zconf.output_module->start(&zconf, &zsend, &zrecv);
	}

	pthread_t *tsend, trecv, tmon;
	int r = pthread_create(&trecv, NULL, start_recv, NULL);
	if (r != 0) {
		ThrowException(Exception::TypeError(String::New("Unable to initialize recieving component")));
	}
	for (;;) {
		pthread_mutex_lock(&recv_ready_mutex);
		if (zconf.recv_ready) {
			pthread_mutex_unlock(&recv_ready_mutex);
			break;
		}
		pthread_mutex_unlock(&recv_ready_mutex);
	}

	tsend = (pthread_t*) xmalloc(zconf.senders * sizeof(pthread_t));
	for (uint8_t i = 0; i < zconf.senders; i++) {
		int sock;
		if (zconf.dryrun) {
			sock = get_dryrun_socket();
		} else {
			sock = get_socket();
		}

		send_arg_t *arg = (send_arg_t*) xmalloc(sizeof(send_arg_t));
		arg->sock = sock;
		arg->shard = get_shard(it, i);
		int r = pthread_create(&tsend[i], NULL, start_send, arg);
		if (r != 0) {
			ThrowException(Exception::TypeError(String::New("Unable to create send thread")));
		}
	}

	lz.drop_privs();

	for (uint8_t i = 0; i < zconf.senders; i++) {
		int r = pthread_join(tsend[i], NULL);
		if (r != 0) {
			ThrowException(Exception::TypeError(String::New("Unable to join send thread")));
		}
	}

	r = pthread_join(trecv, NULL);
	if (r != 0) {
		ThrowException(Exception::TypeError(String::New("Unable to join receive threads")));
	}

	lz.summary();

	if (zconf.output_module && zconf.output_module->close) {
		zconf.output_module->close(&zconf, &zsend, &zrecv);
	}

	if (zconf.probe_module && zconf.probe_module->close) {
		zconf.probe_module->close(&zconf, &zsend, &zrecv);
	}
}

void* libzmap::start_send(void *arg) {
	send_arg_t *v = (send_arg_t *) arg;
	set_cpu();
	send_run(v->sock, v->shard);
	free(v);
	return NULL;
}

void* libzmap::start_recv(void *arg) {
	set_cpu();
	recv_run(&recv_ready_mutex);
	return NULL;
}

#define SI(w,x,y) printf("%s\t%s\t%i\n", w, x, y);
#define SD(w,x,y) printf("%s\t%s\t%f\n", w, x, y);
#define SU(w,x,y) printf("%s\t%s\t%u\n", w, x, y);
#define SLU(w,x,y) printf("%s\t%s\t%lu\n", w, x, (long unsigned int) y);
#define SS(w,x,y) printf("%s\t%s\t%s\n", w, x, y);
#define STRTIME_LEN 1024

void libzmap::summary(void)
{
	char send_start_time[STRTIME_LEN+1];
	assert(dstrftime(send_start_time, STRTIME_LEN, "%c", zsend.start));
	char send_end_time[STRTIME_LEN+1];
	assert(dstrftime(send_end_time, STRTIME_LEN, "%c", zsend.finish));
	char recv_start_time[STRTIME_LEN+1];
	assert(dstrftime(recv_start_time, STRTIME_LEN, "%c", zrecv.start));
	char recv_end_time[STRTIME_LEN+1];
	assert(dstrftime(recv_end_time, STRTIME_LEN, "%c", zrecv.finish));
	double hitrate = ((double) 100 * zrecv.success_unique)/((double)zsend.sent);

	SU("cnf", "target-port", zconf.target_port);
	SU("cnf", "source-port-range-begin", zconf.source_port_first);
	SU("cnf", "source-port-range-end", zconf.source_port_last);
	SS("cnf", "source-addr-range-begin", zconf.source_ip_first);
	SS("cnf", "source-addr-range-end", zconf.source_ip_last);
	SU("cnf", "maximum-targets", zconf.max_targets);
	SU("cnf", "maximum-runtime", zconf.max_runtime);
	SU("cnf", "maximum-results", zconf.max_results);
	SU("cnf", "permutation-seed", zconf.seed);
	SI("cnf", "cooldown-period", zconf.cooldown_secs);
	SS("cnf", "send-interface", zconf.iface);
	SI("cnf", "rate", zconf.rate);
	SLU("cnf", "bandwidth", zconf.bandwidth);
	SU("cnf", "shard-num", (unsigned) zconf.shard_num);
	SU("cnf", "num-shards", (unsigned) zconf.total_shards);
	SU("cnf", "senders", (unsigned) zconf.senders);
	SU("env", "nprocessors", (unsigned) sysconf(_SC_NPROCESSORS_ONLN));
	SS("exc", "send-start-time", send_start_time);
	SS("exc", "send-end-time", send_end_time);
	SS("exc", "recv-start-time", recv_start_time);
	SS("exc", "recv-end-time", recv_end_time);
	SU("exc", "sent", zsend.sent);
	SU("exc", "blacklisted", zsend.blacklisted);
	SU("exc", "first-scanned", zsend.first_scanned);
	SD("exc", "hit-rate", hitrate);
	SU("exc", "success-total", zrecv.success_total);
	SU("exc", "success-unique", zrecv.success_unique);
	SU("exc", "success-cooldown-total", zrecv.cooldown_total);
	SU("exc", "success-cooldown-unique", zrecv.cooldown_unique);
	SU("exc", "failure-total", zrecv.failure_total);
	SU("exc", "sendto-failures", zsend.sendto_failures);
	SU("adv", "permutation-gen", zconf.generator);
	SS("exc", "scan-type", zconf.probe_module->name);
}
