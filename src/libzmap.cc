#include <node.h>
#include <v8.h>

#include "zmap-1.2.1/src/zopt.h"

#include "libzmap.h"

namespace libzmap {

using namespace node;
using namespace v8;

Handle<Value> LibZMAP(const Arguments& args) {
  HandleScope scope;

  Local<Function> callback;
  Local<Object> obj;

  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  if (args[0]->IsFunction()) {
    callback = Local<Function>::Cast(args[0]);
  } else {
    if (!args[1]->IsFunction()) {
      ThrowException(Exception::TypeError(String::New("Function expected as second argument")));
      return scope.Close(Undefined());
    }

    callback = Local<Function>::Cast(args[1]);

    if (!args[0]->IsObject()) {
      ThrowException(Exception::TypeError(String::New("Object expected as first argument")));
      return scope.Close(Undefined());
    }
  }

  if (args[0]->IsObject()) {
    obj = args[0]->ToObject();
  }

  obj->Set(String::NewSymbol("x"),
    String::New("wtf"));

  /* Setup options */
  /* Initialize scan */

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
