#ifndef LIBZMAP_H
#define LIBZMAP_H

#include <node.h>
#include <string>

class libzmap : public node::ObjectWrap {
  public:
    static void Init(v8::Handle<v8::Object> exports);

  protected:
    void Config(v8::Handle<v8::Object> obj);
    void Async(uv_work_t* req);
    void AsyncComplete(uv_work_t* req);
    static v8::Handle<v8::Value> LibZMAP(const v8::Arguments& args);

  private:
    struct Work {
      v8::Persistent<v8::Function> callback;
      bool error;
      std::string error_msg;
    };
};

#endif
