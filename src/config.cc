#include <string.h>
#include <node.h>
#include <v8.h>

#include "./libzmap.h"

extern "C" {
	#include "zmap-1.2.1/lib/logger.h"
	#include "zmap-1.2.1/lib/xalloc.h"
	#include "zmap-1.2.1/lib/blacklist.h"

	#include "zmap-1.2.1/src/types.h"
	#include "zmap-1.2.1/src/aesrand.h"
	#include "zmap-1.2.1/src/zopt.h"
	#include "zmap-1.2.1/src/state.h"
	#include "zmap-1.2.1/src/get_gateway.h"
}

using namespace node;
using namespace v8;

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
	lz.ConfigBlacklist(obj);
	lz.ConfigWhitelist(obj);

	lz.ConfigWhiteBlackLists();

	lz.ConfigTargets();

	lz.ConfigCores();
	lz.ConfigSeed();

}

/*
void libzmap::ConfigLoglevel(Handle<Object> obj) {
	HandleScope scope;

  if (obj->Has(v8::String::NewSymbol("loglevel"))) {
    Handle<v8::Value> value = obj->Get(String::New("loglevel"));
    zconf.log_level = (char*) xmalloc(strlen(*v8::String::Utf8Value(value->ToString())) + 1);
		strcpy(zconf.log_level, *v8::String::Utf8Value(value->ToString()));
  }
	log_debug("loglevel", "%s", zconf.whitelist_filename);
}
*/

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
