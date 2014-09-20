#include <string.h>
#include <node.h>
#include <v8.h>

#include "./libzmap.h"

extern "C" {
#include <unistd.h>
#include <pthread.h>

#include "zmap-1.2.1/lib/xalloc.h"
#include "zmap-1.2.1/lib/blacklist.h"

#include "zmap-1.2.1/src/types.h"
#include "zmap-1.2.1/src/state.h"
#include "zmap-1.2.1/src/zopt.h"
#include "zmap-1.2.1/src/aesrand.h"
#include "zmap-1.2.1/src/get_gateway.h"

#include "zmap-1.2.1/src/probe_modules/probe_modules.h"
#include "zmap-1.2.1/src/output_modules/output_modules.h"
}

using namespace node;
using namespace v8;

pthread_mutex_t cpu_affinity_mutex = PTHREAD_MUTEX_INITIALIZER;

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
	lz.ConfigOutputModule(obj);
	lz.ConfigShards(obj);
	lz.ConfigShardTotal(obj);
	lz.ConfigThreads(obj);

	/* We should always be outputing to stdout as a json object */
	zconf.output_filename = (char*) xmalloc(strlen("-") + 1);
	strcpy(zconf.output_filename, "-");
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
		} else {
			zconf.source_ip_last = zconf.source_ip_first;
		}

	} else {
		struct in_addr default_ip;
		zconf.source_ip_first = (char*) xmalloc(INET_ADDRSTRLEN);
		if (get_iface_ip(zconf.iface, &default_ip) < 0) {
			ThrowException(Exception::TypeError(String::New("Could not detect IP, specify as ipaddr")));
		}
		zconf.source_ip_last = zconf.source_ip_first;
		inet_ntop(AF_INET, &default_ip, zconf.source_ip_first, INET_ADDRSTRLEN);
	}
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
			ThrowException(Exception::TypeError(String::New("Invalid gateway MAC address")));
		}
		zconf.gw_mac_set = 1;
	} else {
		struct in_addr gw_ip;
		if (get_default_gw(&gw_ip, zconf.iface) < 0) {
			ThrowException(Exception::TypeError(String::New("Could not detect gateway MAC address, set using 'mac' param")));
		}
		zconf.gw_ip = gw_ip.s_addr;

		if (get_hw_addr(&gw_ip, zconf.iface, zconf.gw_mac)) {
			ThrowException(Exception::TypeError(String::New("Could not detect gateway MAC address, set using 'mac' param")));
		}
		zconf.gw_mac_set = 1;
	}
}

void libzmap::ConfigRange(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("range"))) {
		Handle<v8::Value> value = obj->Get(String::New("range"));
		zconf.destination_cidrs = (char**) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy((char*) zconf.destination_cidrs, *v8::String::Utf8Value(value->ToString()));
		zconf.destination_cidrs_len = strlen(*v8::String::Utf8Value(value->ToString()));
	}
}

void libzmap::ConfigBlacklist(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("blacklist"))) {
		Handle<v8::Value> value = obj->Get(String::New("blacklist"));
		zconf.blacklist_filename = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(zconf.blacklist_filename, *v8::String::Utf8Value(value->ToString()));
	}
}

void libzmap::ConfigWhitelist(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("whitelist"))) {
		Handle<v8::Value> value = obj->Get(String::New("whitelist"));
		zconf.whitelist_filename = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(zconf.whitelist_filename, *v8::String::Utf8Value(value->ToString()));
	}
}

void libzmap::ConfigWhiteBlackLists(void) {
	HandleScope scope;

	if (blacklist_init(zconf.whitelist_filename, zconf.blacklist_filename,
				zconf.destination_cidrs, zconf.destination_cidrs_len,
				NULL, 0)) {
		ThrowException(Exception::TypeError(String::New("Could not initialize whitelist/blacklists specified")));
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
}

void libzmap::ConfigShardTotal(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("shardtotal"))) {
		Handle<v8::Value> value = obj->Get(String::New("shartotal"));
		zconf.total_shards = value->NumberValue();
	} else {
		zconf.total_shards = 1;
	}
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
		ThrowException(Exception::TypeError(String::New("probe module does not exist")));
	}
}

void libzmap::ConfigOutputModule(Handle<Object> obj) {
	HandleScope scope;
	struct gengetopt_args_info args;

	if (obj->Has(v8::String::NewSymbol("outputmodule"))) {
		Handle<v8::Value> value = obj->Get(String::New("outputmodule"));
		args.output_module_arg = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(args.output_module_arg, *v8::String::Utf8Value(value->ToString()));
	} else {
		args.probe_module_arg = (char*) xmalloc(strlen("csv") + 1);
		strcpy(args.probe_module_arg, "csv");
	}

	zconf.output_module = get_output_module_by_name(args.probe_module_arg);
	zconf.raw_output_fields = (char*) "saddr";
	zconf.filter_duplicates = 1;
	zconf.filter_unsuccessful = 1;

	if (!zconf.probe_module) {
		ThrowException(Exception::TypeError(String::New("output module does not exist")));
	}
}

void libzmap::ConfigBandwidth(Handle<Object> obj) {
	HandleScope scope;

	if (obj->Has(v8::String::NewSymbol("bandwidth"))) {
		Handle<v8::Value> value = obj->Get(String::New("bandwidth"));
		zconf.bandwidth = atoi(*v8::String::Utf8Value(value->ToString()));
		char *suffix = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
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
				ThrowException(Exception::TypeError(String::New("Bandwidth suffix is invalid")));
			}
		}
	}
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
			zconf.senders = max(zconf.senders - 1, 1);
		}
	}

	if (zconf.senders > zsend.targets) {
		zconf.senders = max(zsend.targets, 1);
	}
}

#if defined(__APPLE__)
void libzmap::set_cpu(void) {
	pthread_mutex_lock(&cpu_affinity_mutex);
	static int core=0;
	int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

	mach_port_t tid = pthread_mach_thread_np(pthread_self());
	struct thread_affinity_policy policy;
	policy.affinity_tag = core;
	kern_return_t ret = thread_policy_set(tid,THREAD_AFFINITY_POLICY,
					(thread_policy_t) &policy,THREAD_AFFINITY_POLICY_COUNT);
	if (ret != KERN_SUCCESS) {
		ThrowException(Exception::TypeError(String::New("Cannot set CPU affinity")));
	}
	core = (core + 1) % num_cores;

	pthread_mutex_unlock(&cpu_affinity_mutex);
}

#else

#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/param.h>
#include <sys/cpuset.h>
#define cpu_set_t cpuset_t
#endif

void libzmap::set_cpu(void) {
	pthread_mutex_lock(&cpu_affinity_mutex);
	static int core=0;
	int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);

	if (pthread_setaffinity_np(pthread_self(),
				sizeof(cpu_set_t), &cpuset) != 0) {
		ThrowException(Exception::TypeError(String::New("Can't set thread CPU affinity")));
	}
	core = (core + 1) % num_cores;
	pthread_mutex_unlock(&cpu_affinity_mutex);
}
#endif

void libzmap::drop_privs() {
	HandleScope scope;
	struct passwd *pw;

	if (geteuid() != 0) {
		ThrowException(Exception::TypeError(String::New("Unable to drop privileges")));
	}

	if ((pw = getpwnam("nobody")) != NULL) {
		if (setuid(pw->pw_uid) == 0) {
			return;
		}
	}

	ThrowException(Exception::TypeError(String::New("Unable to drop privileges")));
}

