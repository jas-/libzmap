#include <node.h>
#include <v8.h>

#include "./libzmap.h"

#include "zmap-1.2.1/src/state.h"

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

	/* begin uv_workers to scan */

  return scope.Close(obj);
}

void libzmap::Config(Handle<Object> obj) {
  HandleScope scope;

  if (obj->Has(v8::String::NewSymbol("blacklist"))) {
    Handle<v8::Value> value = obj->Get(String::New("blacklist"));
    zconf.blacklist_filename = *v8::String::Utf8Value(value->ToString());
  }
}
