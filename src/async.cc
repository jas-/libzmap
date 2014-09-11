#include <node.h>
#include <v8.h>

#include "./libzmap.h"

#include "zmap-1.2.1/src/state.h"

using namespace node;
using namespace v8;

void libzmap::Async(uv_async_t* req, int status) {
	//thread_comm_t comm;

	//uv_async_init(uv_default_loop(), &comm.async, AsyncCallback);
}

void libzmap::AsyncCallback(uv_async_t* req, int status) {

}

void libzmap::Listener(uv_async_t* handle, int status) {

}
