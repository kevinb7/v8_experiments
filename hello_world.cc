#include "include/v8.h"
#include "include/libplatform/libplatform.h"
#include <iostream>
#include <string>
#include <Python.h>

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

  isolate->SetCaptureStackTraceForUncaughtExceptions(true);
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

  TryCatch trycatch(isolate);
  Local<Value> value = script->Run();
  if (value.IsEmpty()) {
    Local<Value> exception = trycatch.Exception();
    String::Utf8Value exception_str(exception);
    printf("Caught and exception: %s\n", *exception_str);
    Local<Message> message = trycatch.Message();
    if (message.IsEmpty()) {
      // V8 didn't provide any extra information about this error; just
      // print the exception.
      printf("%s\n", *exception_str);
    } else {
      printf("line number: %d\n", message->GetLineNumber());

      Local<StackTrace>stack_trace = message->GetStackTrace();
      if (stack_trace.IsEmpty()) {
        printf("no stack trace info.\n");
      } else {
        printf("frame count = %d\n", stack_trace->GetFrameCount());
        for (int i = 0; i < stack_trace->GetFrameCount(); i++) {
          Local<StackFrame> frame = stack_trace->GetFrame(i);
          String::Utf8Value temp_str(frame->GetFunctionName());
          printf("%d:%d in function name: %s\n", frame->GetLineNumber(), frame->GetColumn(), *temp_str);
        }
      }
    }
    return NULL;
  } else {
    String::Utf8Value utf8_value(value);
    std::string result_string = std::string(*utf8_value);
    return result_string.c_str();
  } 
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

extern "C" int print_date() {
  // Py_SetProgramName(argv[0]);  /* optional but recommended */
  Py_Initialize();
  PyRun_SimpleString("from time import time,ctime\n"
                     "print 'Today is',ctime(time())\n");
  Py_Finalize();
  return 0;
}

extern "C" static PyObject *spam_system(PyObject *self, PyObject *args) {
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    return Py_BuildValue("i", sts);
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
