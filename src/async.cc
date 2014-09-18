#include <node.h>
#include <v8.h>

#include "./libzmap.h"

extern "C" {
#include <pcap/pcap.h>
#include <pthread.h>

#include "zmap-1.2.1/lib/xalloc.h"

#include "zmap-1.2.1/lib/logger.h"
#include "zmap-1.2.1/src/state.h"
#include "zmap-1.2.1/src/iterator.h"
#include "zmap-1.2.1/src/recv.h"
#include "zmap-1.2.1/src/send.h"
}

using namespace node;
using namespace v8;

pthread_mutex_t recv_ready_mutex = PTHREAD_MUTEX_INITIALIZER;

void libzmap::Async(uv_async_t* req, int status) {

}

void libzmap::AsyncCallback(uv_async_t* req, int status) {

}

void libzmap::Listener(uv_async_t* handle, int status) {

}

void libzmap::Pthreads(void) {
	libzmap lz;
	iterator_t *it = send_init();

	if (!it) {
		log_fatal("zmap", "unable to initialize sending component");
	}

	pthread_t *tsend, trecv, tmon;
	int r = pthread_create(&trecv, NULL, start_recv, NULL);
	if (r != 0) {
		log_fatal("zmap", "unable to create recv thread");
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
			log_fatal("zmap", "unable to create send thread");
			exit(EXIT_FAILURE);
		}
	}
	log_debug("zmap", "%d sender threads spawned", zconf.senders);

	lz.drop_privs();

	for (uint8_t i = 0; i < zconf.senders; i++) {
		int r = pthread_join(tsend[i], NULL);
		if (r != 0) {
			log_fatal("zmap", "unable to join send thread");
		}
	}
	log_debug("zmap", "senders finished");
	r = pthread_join(trecv, NULL);
	if (r != 0) {
		log_fatal("zmap", "unable to join recv thread");
	}
	/* Set quiet arg */
	zconf.quiet = 0;
	if (!zconf.quiet) {
		pthread_join(tmon, NULL);
		if (r != 0) {
			log_fatal("zmap", "unable to join monitor thread");
		}
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
