#ifndef LIBZMAP_H
#define LIBZMAP_H

#include <node.h>
#include <string>

class libzmap : public node::ObjectWrap {
  public:
    static void Init(v8::Handle<v8::Object> exports);

  protected:
    uv_async_t watcher;

    void Config(v8::Handle<v8::Object> obj);
    void Async(uv_async_t* req, int status);
    void AsyncComplete(uv_async_t* req);
    void Listener(uv_async_t* handle, int status);

    static v8::Handle<v8::Value> LibZMAP(const v8::Arguments& args);

  private:
    uv_async_t async;

    struct Work {
      bool error;
      std::string error_msg;
      v8::Persistent<v8::Function> callback;
    };
};

#endif
