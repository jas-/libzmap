#ifndef LIBZMAP_H
#define LIBZMAP_H

#include <node.h>
#include <string>

extern "C" {
#include "zmap-1.2.1/src/types.h"
}

class libzmap : public node::ObjectWrap {
	public:
		static void Init(v8::Handle<v8::Object> exports);

	protected:
		uv_async_t watcher;

		int max(int a, int b);
		int parse_mac(macaddr_t *out, char *in);

		void Config(v8::Handle<v8::Object> obj);
		void ConfigLoglevel(v8::Handle<v8::Object> obj);
		void ConfigIface(v8::Handle<v8::Object> obj);
		void ConfigIpaddr(v8::Handle<v8::Object> obj);
		void ConfigHwaddr(v8::Handle<v8::Object> obj);
		void ConfigRange(v8::Handle<v8::Object> obj);
		void ConfigBlacklist(v8::Handle<v8::Object> obj);
		void ConfigWhitelist(v8::Handle<v8::Object> obj);

		void ConfigWhiteBlackLists(void);
		void ConfigTargets(void);
		void ConfigCores(void);
		void ConfigSeed(void);

		void Async(uv_async_t* req, int status);
		void AsyncCallback(uv_async_t* req, int status);
		void Listener(uv_async_t* handle, int status);

		static v8::Handle<v8::Value> LibZMAP(const v8::Arguments& args);

	private:
		typedef struct {
			uv_async_t async;
			uv_mutex_t mutex;
			uv_thread_t thread;
		} thread_comm_t;
};

#endif
