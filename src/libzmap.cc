#include <node.h>
#include <v8.h>

#include "./libzmap.h"

extern "C" {
#include "zmap-1.2.1/src/state.h"
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

  lz.Threads();

  /* return NULL error & JSON object to specified callback param */

	return scope.Close(Undefined());
}

void libzmap::Init (Handle<Object> exports) {
	exports->Set(String::NewSymbol("zmap"),
			FunctionTemplate::New(LibZMAP)->GetFunction());
}

extern "C" {
	NODE_MODULE(zmap, libzmap::Init)
}
