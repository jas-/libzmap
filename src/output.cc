extern "C" {
#include <unistd.h>
#include "zmap-1.2.1/lib/logger.h"
#include "zmap-1.2.1/src/state.h"
#include "zmap-1.2.1/src/fieldset.h"
#include "zmap-1.2.1/src/probe_modules/probe_modules.h"
#include "module.h"
}

#include "libzmap.h"

#define STRTIME_LEN 1024

using namespace node;
using namespace v8;

Handle<Value> libzmap::Summary(void) {
	HandleScope scope;

	Local<Object> obj = Object::New();
	Local<Object> cnf = Object::New();
	Local<Object> stat = Object::New();

	char send_start_time[STRTIME_LEN+1];
	assert(dstrftime(send_start_time, STRTIME_LEN, "%c", zsend.start));
	char send_end_time[STRTIME_LEN+1];
	assert(dstrftime(send_end_time, STRTIME_LEN, "%c", zsend.finish));
	char recv_start_time[STRTIME_LEN+1];
	assert(dstrftime(recv_start_time, STRTIME_LEN, "%c", zrecv.start));
	char recv_end_time[STRTIME_LEN+1];
	assert(dstrftime(recv_end_time, STRTIME_LEN, "%c", zrecv.finish));
	double hitrate = ((double) 100 * zrecv.success_unique)/((double)zsend.sent);

	cnf->Set(String::NewSymbol("scan-type"),
					 String::New(zconf.probe_module->name));
	cnf->Set(String::NewSymbol("interface"), String::New(zconf.iface));
	cnf->Set(String::NewSymbol("processors"),
					 Uint32::New((unsigned) sysconf(_SC_NPROCESSORS_ONLN)));
	cnf->Set(String::NewSymbol("target-port"), Uint32::New(zconf.target_port));
	cnf->Set(String::NewSymbol("source-address-start"),
					 String::New(zconf.source_ip_first));
	cnf->Set(String::NewSymbol("source-address-end"),
					 String::New(zconf.source_ip_last));
	cnf->Set(String::NewSymbol("source-port-start"),
					 Uint32::New(zconf.source_port_first));
	cnf->Set(String::NewSymbol("source-port-end"),
					 Uint32::New(zconf.source_port_last));
	cnf->Set(String::NewSymbol("max-targets"), Uint32::New(zconf.max_targets));
	cnf->Set(String::NewSymbol("max-runtime"), Uint32::New(zconf.max_runtime));
	cnf->Set(String::NewSymbol("max-results"), Uint32::New(zconf.max_results));
	cnf->Set(String::NewSymbol("bandwidth"), Uint32::New(zconf.bandwidth));
	cnf->Set(String::NewSymbol("rate"), Uint32::New(zconf.rate));

	stat->Set(String::NewSymbol("hit-rate"), Uint32::New(hitrate));
	stat->Set(String::NewSymbol("total-sent"), Uint32::New(zsend.sent));
	stat->Set(String::NewSymbol("total-blacklisted"),
						Uint32::New(zsend.blacklisted));
	stat->Set(String::NewSymbol("successful-total"),
						Uint32::New(zrecv.success_total));
	stat->Set(String::NewSymbol("successful-unique"),
						Uint32::New(zrecv.success_unique));
	stat->Set(String::NewSymbol("failure-total"),
						Uint32::New(zrecv.failure_total));
	stat->Set(String::NewSymbol("send-started"), String::New(send_start_time));
	stat->Set(String::NewSymbol("send-finished"), String::New(send_end_time));
	stat->Set(String::NewSymbol("recv-started"), String::New(recv_start_time));
	stat->Set(String::NewSymbol("recv-finished"), String::New(recv_end_time));

	obj->Set(String::NewSymbol("configuration"), cnf);
	obj->Set(String::NewSymbol("statistics"), stat);

//	if (zsend.complete)
//		return_object();

	return scope.Close(obj);
}
