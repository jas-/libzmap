#include <node.h>
#include <v8.h>

#include "./libzmap.h"

using namespace node;
using namespace v8;

void libzmap::Init (Handle<Object> exports) {
	exports->Set(String::NewSymbol("zmap"),
			FunctionTemplate::New(LibZMAP)->GetFunction());
}

extern "C" {
	NODE_MODULE(zmap, libzmap::Init)
}
