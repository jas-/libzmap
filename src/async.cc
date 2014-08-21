/*
 * libzmap
 *
 * Copyright 2014 Jason Gerfen
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <node.h>
#include <nan.h>
#include "./scan.h"
#include "./async.h"

using v8::Function;
using v8::Local;
using v8::Null;
using v8::Number;
using v8::Value;
using v8::Handle;
using v8::Object;
using v8::String;

class ScanWorker : public NanAsyncWorker {
 public:
  ScanWorker(NanCallback *callback, Handle<Object> args)
    : NanAsyncWorker(callback) {}
  ~ScanWorker() {}

  void Execute () {
    scan = Scan(args);
  }

  void HandleOKCallback () {
    NanScope();

    Local<Value> argv[] = {
        NanNull()
      , NanNew<v8::Object>(scan)
    };

    callback->Call(2, argv);
  }

	private:
		Local<Object> scan;
		Local<Object> args;
};

NAN_METHOD(AsyncScan) {
  NanScope();

	if (args.Length() < 2) {
		return NanThrowTypeError("An object & callback function required");
	}

  NanCallback *callback = new NanCallback(args[0].As<Function>());
	Local<Object> obj = args[1].As<Object>();

  NanAsyncQueueWorker(new ScanWorker(callback, args));
  NanReturnUndefined();
}
