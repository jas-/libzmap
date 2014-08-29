#include <node.h>
#include <nan.h>

#include "libzmap.h"

using namespace v8;

namespace libzmap {

NAN_METHOD(LibZMAP) {
	NanScope();

  NanReturnValue(NanNew<String>("hello world"));
}

void Init (v8::Handle<v8::Object> target) {

	v8::Local<v8::Function> libzmap =
		NanNew<v8::FunctionTemplate>(LibZMAP)->GetFunction();

	target->Set(NanNew("libzmap"), libzmap);
}

NODE_MODULE(libzmap, Init)

} // namespace libzmap
