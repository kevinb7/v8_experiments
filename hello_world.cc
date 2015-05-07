#include "include/v8.h"
#include "include/libplatform/libplatform.h"
#include <iostream>
#include <string>

using namespace v8;

class ShellArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
 public:
  virtual void* Allocate(size_t length) {
    void* data = AllocateUninitialized(length);
    return data == NULL ? data : memset(data, 0, length);
  }
  virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
  virtual void Free(void* data, size_t) { free(data); }
};

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

v8::Isolate* isolate;
Platform* _platform;
Persistent<Context> context_;

extern "C" void init() {
  // Initialize V8.
  V8::InitializeICU();
  _platform = platform::CreateDefaultPlatform();
  V8::InitializePlatform(_platform);
  V8::Initialize();
  // v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

  // Create a new Isolate and make it the current one.
  ShellArrayBufferAllocator *array_buffer_allocator = new ShellArrayBufferAllocator();
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = array_buffer_allocator;
  isolate = v8::Isolate::New(create_params);

  HandleScope handle_scope(isolate);

  v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
  global->Set(v8::String::NewFromUtf8(isolate, "print"),
              v8::FunctionTemplate::New(isolate, Print));
  v8::Handle<v8::Context> context = Context::New(isolate, NULL, global);
  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
  }
  context_.Reset(isolate, context);
}

extern "C" const char *run(const char *src) {
  Isolate::Scope isolate_scope(isolate);

  // Create a stack-allocated handle scope.
  HandleScope handle_scope(isolate);

  // Create a new context.
  // context = Context::New(isolate, NULL, global);
  v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, context_);

  // Enter the context for compiling and running the hello world script.
  Context::Scope context_scope(context);

  Local<String> source = String::NewFromUtf8(isolate, src);
  Local<Script> script = Script::Compile(source);
  Local<Value> result = script->Run();

  // Convert the result to an UTF8 string and print it.
  // String::Utf8Value utf8_b(result);
  // printf("%s\n", *utf8_b);

  String::Utf8Value utf8(result);
  std::string from = std::string(*utf8);
  return from.c_str();
}

extern "C" void cleanup() {
  HandleScope handle_scope(isolate);

  context_.Reset();
  // Dispose the isolate and tear down V8.
  // isolate->Dispose();
  V8::Dispose();
  V8::ShutdownPlatform();
  delete _platform;
}

int main(int argc, char* argv[]) {
  init();

  run("print ('hello, world');");
  run("var i = 1;");
  run("i += 1;");
  run("print ('i = ' + i);");

  cleanup();

  return 0;
}
