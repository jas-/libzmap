#include <string.h>
#include <node.h>
#include <v8.h>

#include "./libzmap.h"

extern "C" {

#include <pcap/pcap.h>
#include <pthread.h>

#include "zmap-1.2.1/lib/logger.h"
#include "zmap-1.2.1/lib/xalloc.h"
#include "zmap-1.2.1/lib/blacklist.h"

#include "zmap-1.2.1/src/types.h"
#include "zmap-1.2.1/src/state.h"
#include "zmap-1.2.1/src/zopt.h"
#include "zmap-1.2.1/src/aesrand.h"
#include "zmap-1.2.1/src/send.h"
#include "zmap-1.2.1/src/get_gateway.h"
#include "zmap-1.2.1/src/recv.h"
#include "zmap-1.2.1/src/iterator.h"
#include "zmap-1.2.1/src/probe_modules/probe_modules.h"
}

using namespace node;
using namespace v8;

pthread_mutex_t cpu_affinity_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_ready_mutex = PTHREAD_MUTEX_INITIALIZER;

Handle<Value> libzmap::LibZMAP(const Arguments& args) {
	HandleScope scope;
	libzmap lz;

	Local<Function> callback;
	Local<Object> obj;

	if (args.Length() < 1) {
		ThrowException(Exception::TypeError(String::New("Arguments invalid")));
		return scope.Close(Undefined());
	}

	if (args[0]->IsFunction()) {
		callback = Local<Function>::Cast(args[0]);
	} else {
		if (!args[1]->IsFunction()) {
			ThrowException(Exception::TypeError(String::New("Function expected")));
			return scope.Close(Undefined());
		}

		callback = Local<Function>::Cast(args[1]);

		if (!args[0]->IsObject()) {
			ThrowException(Exception::TypeError(String::New("Object expected")));
			return scope.Close(Undefined());
		}
	}

	if (args[0]->IsObject()) {
		obj = args[0]->ToObject();
		lz.Config(obj);
	}

	return scope.Close(obj);
}

int libzmap::max(int a, int b) {
	if (a >= b) {
		return a;
	}
	return b;
}

void libzmap::Config(Handle<Object> obj) {
	HandleScope scope;
	libzmap lz;

	lz.ConfigIface(obj);
	lz.ConfigIpaddr(obj);
	lz.ConfigHwaddr(obj);
	lz.ConfigRange(obj);
	lz.ConfigBandwidth(obj);
	lz.ConfigBlacklist(obj);
	lz.ConfigWhitelist(obj);

	lz.ConfigWhiteBlackLists();
	lz.ConfigTargets();
	lz.ConfigCores();
	lz.ConfigSeed();

	lz.ConfigProbeModule(obj);
	lz.ConfigShards(obj);
	lz.ConfigShardTotal(obj);
	lz.ConfigThreads(obj);

	iterator_t *it = send_init();
	if (!it) {
		log_fatal("zmap", "unable to initialize sending component");
	}

	// start threads
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


	/* start threads, use uv_async here vs. pthreads */
	/* drop root privs */
	/* async callback for completed workers */

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

void libzmap::ConfigLoglevel(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("loglevel"))) {
		Handle<v8::Value> value = obj->Get(String::New("loglevel"));
//		zconf.log_level = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
//		strcpy(zconf.log_level, *v8::String::Utf8Value(value->ToString()));
	}
	log_debug("loglevel", "%s", zconf.whitelist_filename);
}

void libzmap::ConfigIface(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("iface"))) {
		Handle<v8::Value> value = obj->Get(String::New("iface"));
		zconf.iface = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(zconf.iface, *v8::String::Utf8Value(value->ToString()));
	} else {
		zconf.iface = get_default_iface();
		assert(zconf.iface);
	}
	log_debug("iface", "%s", zconf.iface);
}

void libzmap::ConfigIpaddr(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("ipaddr"))) {
		Handle<v8::Value> value = obj->Get(String::New("ipaddr"));
		zconf.source_ip_first = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(zconf.source_ip_first, *v8::String::Utf8Value(value->ToString()));

		char *dash = strchr(zconf.source_ip_first, '-');

		if (dash) {
			*dash = '\0';
			zconf.source_ip_last = dash+1;
		}

	} else {
		struct in_addr default_ip;
		zconf.source_ip_first = (char*) xmalloc(INET_ADDRSTRLEN);
		zconf.source_ip_last = zconf.source_ip_first;
		if (get_iface_ip(zconf.iface, &default_ip) < 0) {
			log_fatal("zmap", "could not detect default IP address for for %s."
					" Try specifying a source address (-S).", zconf.iface);
		}
		inet_ntop(AF_INET, &default_ip, zconf.source_ip_first, INET_ADDRSTRLEN);
		log_debug("zmap", "no source IP address given. will use default address: %s.",
				zconf.source_ip_first);
	}
	log_debug("ipaddr", "%s:%s", zconf.source_ip_first, zconf.source_ip_last);
}

#define MAC_LEN ETHER_ADDR_LEN
int libzmap::parse_mac(macaddr_t *out, char *in)
{
	if (strlen(in) < MAC_LEN*3-1)
		return 0;

	char octet[4];
	octet[2] = '\0';
	for (int i=0; i < MAC_LEN; i++) {
		if (i < MAC_LEN-1 && in[i*3+2] != ':') {
			return 0;
		}
		strncpy(octet, &in[i*3], 2);
		char *err = NULL;
		long b = strtol(octet, &err, 16);
		if (err && *err != '\0') {
			return 0;
		}
		out[i] = b & 0xFF;
	}
	return 1;
}

void libzmap::ConfigHwaddr(Handle<Object> obj) {
	HandleScope scope;
	libzmap lz;
	struct gengetopt_args_info args;

	if (obj->Has(v8::String::NewSymbol("mac"))) {
		Handle<v8::Value> value = obj->Get(String::New("mac"));
		args.gateway_mac_arg = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(args.gateway_mac_arg, *v8::String::Utf8Value(value->ToString()));
		if (!lz.parse_mac(zconf.gw_mac, args.gateway_mac_arg)) {
			log_error("zmap", "invalid MAC address `%s'\n", args.gateway_mac_arg);
		}
		zconf.gw_mac_set = 1;
	} else {
		struct in_addr gw_ip;
		if (get_default_gw(&gw_ip, zconf.iface) < 0) {
			log_fatal("zmap", "could not detect default gateway address for %s."
					" Try setting default gateway mac address (-G).",
					zconf.iface);
		}
		log_debug("zmap", "found gateway IP %s on %s", inet_ntoa(gw_ip), zconf.iface);
		zconf.gw_ip = gw_ip.s_addr;

		if (get_hw_addr(&gw_ip, zconf.iface, zconf.gw_mac)) {
			log_fatal("zmap", "could not detect GW MAC address for %s on %s."
					" Try setting default gateway mac address (-G), or run"
					" \"arp <gateway_ip>\" in terminal.",
					inet_ntoa(gw_ip), zconf.iface);
		}
		zconf.gw_mac_set = 1;
	}
	log_debug("mac", "%02x:%02x:%02x:%02x:%02x:%02x",
			zconf.gw_mac[0], zconf.gw_mac[1], zconf.gw_mac[2],
			zconf.gw_mac[3], zconf.gw_mac[4], zconf.gw_mac[5]);
}

void libzmap::ConfigRange(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("range"))) {
		Handle<v8::Value> value = obj->Get(String::New("range"));
		zconf.destination_cidrs = (char**) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy((char*) zconf.destination_cidrs, *v8::String::Utf8Value(value->ToString()));
		zconf.destination_cidrs_len = strlen(*v8::String::Utf8Value(value->ToString()));
	}
	log_debug("range", "%s", zconf.destination_cidrs);
}

void libzmap::ConfigBlacklist(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("blacklist"))) {
		Handle<v8::Value> value = obj->Get(String::New("blacklist"));
		zconf.blacklist_filename = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(zconf.blacklist_filename, *v8::String::Utf8Value(value->ToString()));
	}
	log_debug("blacklist", "%s", zconf.blacklist_filename);
}

void libzmap::ConfigWhitelist(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("whitelist"))) {
		Handle<v8::Value> value = obj->Get(String::New("whitelist"));
		zconf.whitelist_filename = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(zconf.whitelist_filename, *v8::String::Utf8Value(value->ToString()));
	}
	log_debug("whitelist", "%s", zconf.whitelist_filename);
}

void libzmap::ConfigWhiteBlackLists(void) {
	HandleScope scope;

	if (blacklist_init(zconf.whitelist_filename, zconf.blacklist_filename,
				zconf.destination_cidrs, zconf.destination_cidrs_len,
				NULL, 0)) {
		log_fatal("zmap", "unable to initialize blacklist / whitelist");
	}
}

void libzmap::ConfigTargets(void) {
	HandleScope scope;

	uint64_t allowed = blacklist_count_allowed();
	assert(allowed <= (1LL << 32));
	if (allowed == (1LL << 32)) {
		zsend.targets = 0xFFFFFFFF;
	} else {
		zsend.targets = allowed;
	}
	if (zsend.targets > zconf.max_targets) {
		zsend.targets = zconf.max_targets;
	}
}

void libzmap::ConfigShards(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("shards"))) {
		Handle<v8::Value> value = obj->Get(String::New("shards"));
		zconf.shard_num = value->NumberValue();
	} else {
		zconf.shard_num = 0;
	}
	log_debug("shards", "%d", zconf.shard_num);
}

void libzmap::ConfigShardTotal(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("shardtotal"))) {
		Handle<v8::Value> value = obj->Get(String::New("shartotal"));
		zconf.total_shards = value->NumberValue();
	} else {
		zconf.total_shards = 1;
	}
	log_debug("shardtotal", "%d", zconf.total_shards);
}

void libzmap::ConfigCores(void) {
	HandleScope scope;
	libzmap lz;

	int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
	zconf.senders = lz.max(num_cores - 1, 1);
	if (!zconf.quiet) {
		zconf.senders = lz.max(zconf.senders - 1, 1);
	}

	if (zconf.senders > zsend.targets) {
		zconf.senders = max(zsend.targets, 1);
	}
}

void libzmap::ConfigSeed(void) {
	HandleScope scope;

	if (zconf.use_seed) {
		aesrand_init(zconf.seed + 1);
	} else {
		aesrand_init(0);
	}
}

void libzmap::ConfigProbeModule(Handle<Object> obj) {
	HandleScope scope;
	struct gengetopt_args_info args;

	if (obj->Has(v8::String::NewSymbol("probemodule"))) {
		Handle<v8::Value> value = obj->Get(String::New("probemodule"));
		args.probe_module_arg = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(args.probe_module_arg, *v8::String::Utf8Value(value->ToString()));
	} else {
		args.probe_module_arg = (char*) xmalloc(strlen("icmp_echoscan") + 1);
		strcpy(args.probe_module_arg, "icmp_echoscan");
	}

	zconf.probe_module = get_probe_module_by_name(args.probe_module_arg);
	if (!zconf.probe_module) {
		log_fatal("zmap", "specified probe module (%s) does not exist",
				args.probe_module_arg);
	  exit(EXIT_FAILURE);
	}
	log_debug("probemodule", "%s", zconf.probe_module);
}

void libzmap::ConfigOutputModule(Handle<Object> obj) {
	HandleScope scope;

}

void libzmap::ConfigBandwidth(Handle<Object> obj) {
	HandleScope scope;
	struct gengetopt_args_info args;
	char *suffix;

	if (obj->Has(v8::String::NewSymbol("bandwidth"))) {
		Handle<v8::Value> value = obj->Get(String::New("bandwidth"));
		zconf.bandwidth = atoi(*v8::String::Utf8Value(value->ToString()));
		suffix = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(suffix, *v8::String::Utf8Value(value->ToString()));

		while (*suffix >= '0' && *suffix <= '9') {
			suffix++;
		}

		if (*suffix) {
			switch (*suffix) {
			case 'G': case 'g':
				zconf.bandwidth *= 1000000000;
				break;
			case 'M': case 'm':
				zconf.bandwidth *= 1000000;
				break;
			case 'K': case 'k':
				zconf.bandwidth *= 1000;
				break;
			default:
				log_fatal("bandwidth", "Unknown bandwidth suffix %s", suffix);
			}
		}
	}

	log_debug("bandwidth", "%d%s", zconf.bandwidth, suffix);
}

void libzmap::ConfigThreads(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("threads"))) {
		Handle<v8::Value> value = obj->Get(String::New("threads"));
		zconf.senders = value->NumberValue();
	} else {
		int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
		zconf.senders = max(num_cores - 1, 1);
		if (!zconf.quiet) {
			// If monitoring, save a core for the monitor thread
			zconf.senders = max(zconf.senders - 1, 1);
		}
	}

	if (zconf.senders > zsend.targets) {
		zconf.senders = max(zsend.targets, 1);
	}

	log_debug("threads", "%d", zconf.senders);
}

#if defined(__APPLE__)
void libzmap::set_cpu(void)
{
	pthread_mutex_lock(&cpu_affinity_mutex);
	static int core=0;
	int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

	mach_port_t tid = pthread_mach_thread_np(pthread_self());
	struct thread_affinity_policy policy;
	policy.affinity_tag = core;
	kern_return_t ret = thread_policy_set(tid,THREAD_AFFINITY_POLICY,
					(thread_policy_t) &policy,THREAD_AFFINITY_POLICY_COUNT);
	if (ret != KERN_SUCCESS) {
		log_error("zmap", "can't set thread CPU affinity");
	}
	log_trace("zmap", "set thread %u affinity to core %d",
			pthread_self(), core);
	core = (core + 1) % num_cores;

	pthread_mutex_unlock(&cpu_affinity_mutex);
}

#else

#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/param.h>
#include <sys/cpuset.h>
#define cpu_set_t cpuset_t
#endif

void libzmap::set_cpu(void)
{
	pthread_mutex_lock(&cpu_affinity_mutex);
	static int core=0;
	int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);

	if (pthread_setaffinity_np(pthread_self(),
				sizeof(cpu_set_t), &cpuset) != 0) {
		log_error("zmap", "can't set thread CPU affinity");
	}
	log_trace("zmap", "set thread %u affinity to core %d",
			pthread_self(), core);
	core = (core + 1) % num_cores;
	pthread_mutex_unlock(&cpu_affinity_mutex);
}
#endif
