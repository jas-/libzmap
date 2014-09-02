#include <node.h>
#include <v8.h>

#include "zmap-1.2.1/src/zopt.h"

#include "libzmap.h"

namespace libzmap {

using namespace node;
using namespace v8;

Handle<Value> LibZMAP(const Arguments& args) {
  HandleScope scope;

  struct gengetopt_args_info argv;
  struct cmdline_parser_params *params;
  params = cmdline_parser_params_create();
  params->initialize = 1;
  params->override = 0;
  params->check_required = 0;

  int config_loaded = 0;

  if (cmdline_parser_ext(args.Length(), args[0], &argv, params) != 0) {
    ThrowException(Exception::TypeError(String::New("cmdline_parser_ext met")));
    //exit(EXIT_SUCCESS);
  }

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
  }

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
