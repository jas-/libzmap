#include <node.h>
#include <v8.h>

#include "libzmap.h"

namespace libzmap {

using namespace node;
using namespace v8;

Handle<Value> LibZMAP(const Arguments& args) {
  HandleScope scope;

  Local<Object> obj = Object::New();
  obj->Set(String::NewSymbol("x"), String::New("hello"));
  obj->Set(String::NewSymbol("y"), String::New("world"));

  return scope.Close(obj);
}

void Init (Handle<Object> exports) {
  exports->Set(String::NewSymbol("zmap"),
    FunctionTemplate::New(LibZMAP)->GetFunction());
}

extern "C" {
  NODE_MODULE(zmap, Init)
}

} // namespace libzmap
