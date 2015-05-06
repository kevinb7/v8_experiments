#include "include/v8.h"
#include "include/libplatform/libplatform.h"
#include <iostream>

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

int main(int argc, char* argv[]) {
  // Initialize V8.
  V8::InitializeICU();
  Platform* platform = platform::CreateDefaultPlatform();
  V8::InitializePlatform(platform);
  V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

  // Create a new Isolate and make it the current one.
  ShellArrayBufferAllocator array_buffer_allocator;
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = &array_buffer_allocator;
  v8::Isolate* isolate = v8::Isolate::New(create_params);

  {
    Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    HandleScope handle_scope(isolate);

    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    global->Set(v8::String::NewFromUtf8(isolate, "print"),
                v8::FunctionTemplate::New(isolate, Print));

    // Create a new context.
    Local<Context> context = Context::New(isolate, NULL, global);

    if (context.IsEmpty()) {
      fprintf(stderr, "Error creating context\n");
      return 1;
    }

    // Enter the context for compiling and running the hello world script.
    Context::Scope context_scope(context);

    // Create a string containing the JavaScript source code.
    Local<String> source = String::NewFromUtf8(isolate, "print('Hello' + ', World!');");

    // Compile the source code.
    Local<Script> script = Script::Compile(source);

    // Run the script to get the result.
    Local<Value> result = script->Run();

    // Convert the result to an UTF8 string and print it.
    // String::Utf8Value utf8(result);
    // printf("%s\n", *utf8);

    // Run the script to get the result.
    source = String::NewFromUtf8(isolate, "var i = 1;");
    script = Script::Compile(source);
    result = script->Run();

    source = String::NewFromUtf8(isolate, "print('i = ' + i);");
    script = Script::Compile(source);
    result = script->Run();

    // Convert the result to an UTF8 string and print it.
    // String::Utf8Value utf8_b(result);
    // printf("%s\n", *utf8_b);
  }
  
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  V8::Dispose();
  V8::ShutdownPlatform();
  delete platform;
  return 0;
}
