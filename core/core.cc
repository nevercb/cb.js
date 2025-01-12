#include "core.h"

void No::Core::RegisterBuiltins(Isolate * isolate, Local<Object> No) {
    Local<Object> target = Object::New(isolate);
    // 把新对象传入各个模块中，各个模块会往 target 对象挂载导出的功能
    // ObjectSet 可能会将 target 转换为一个持久句柄（Persistent）或直接挂载到 No 对象。
    // 这样，target 的引用被 No 保存，确保对象不会被垃圾回收。
    // 如果没有其他引用：
    // 当 target 超出作用域且不再被引用时，V8 的垃圾回收器会销毁这个对象。
    Process::Init(isolate, target); 
    Console::Init(isolate, target);
    Loader::Init(isolate, target);
    FS::Init(isolate, target);
    Util::Init(isolate, target);
    VM::Init(isolate, target);
    TCP::Init(isolate, target);
    Pipe::Init(isolate, target);
    UDP::Init(isolate, target);
    Timer::Init(isolate, target);
    HTTP::Init(isolate, target);
    DNS::Init(isolate, target);
    Worker::Init(isolate, target);
    Message::Init(isolate, target);
    MicroTask::Init(isolate, target);
    Immediate::Init(isolate, target);
    FSWatcher::Init(isolate, target);
    ChildProcess::Init(isolate, target);
    OS::Init(isolate, target);
    Signal::Init(isolate, target);
    Addon::Init(isolate, target);
    Buffer::Init(isolate, target);
    UV::Init(isolate, target);
    ObjectSet(isolate, No, "buildin", target);
}

static std::string debugload(const char * filename) {
    int fd = open(filename, 0, O_RDONLY);
    if (fd == -1) {
        return "";
    }
    std::string code;
    char buffer[4096];
    while (1)
    {
        memset(buffer, 0, 4096);
        int ret = read(fd, buffer, 4096);
        if (ret == -1) {
            return "";
        }
        if (ret == 0) {
            break;
        }
        code.append(buffer, ret);
    }
    close(fd);
    return code;
}

void No::Core::Run(Environment * env) {
    // 一个isolate可能包含多个context
    // Isolate 是 V8 中的一个独立的 JavaScript 引擎实例，代表了一个隔离的 JavaScript 执行环境。每个 Isolate 是完全独立的，拥有自己的堆内存和垃圾回收机制。
    // Context 是 JavaScript 代码的执行作用域，它定义了变量、函数和对象的可见性。Context 通常依赖于某个 Isolate，因为它属于某个隔离环境。
    Isolate * isolate = env->GetIsolate();
    isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
    Local<Context> context = env->GetContext();
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Context::Scope context_scope(context);
    // 创建一个对象
    // 这个 No 对象是一个 JavaScript 对象，存在于 V8 的 Isolate 和 Context 中。它可以用来存储各种功能或数据，并将这些内容暴露给 JavaScript 层。
    Local<Object> No = Object::New(isolate);
    // 往No 这个对象上注册各种能力，得到的No对象上会包含模块信息
    RegisterBuiltins(isolate, No);
    Local<Object> global = context->Global();
    // 在 C++ 层，No 对象被传递到 JavaScript 的全局作用域中，以便 JS 层可以访问它
    global->Set(context, NewString(isolate, "global"), global).Check();
    // 加载 No.js 这个文件，进行js层面的初始化
    const char* filename = "No.js";
    // 这段代码是使用 V8 引擎 API 创建一个 ScriptOrigin 对象的代码片段，主要用于指定 JavaScript 脚本的来源信息（如文件名、行号等）。这在嵌入式 JavaScript 环境中很常见，例如 Node.js 或其他使用 V8 的嵌入式应用程序。
    ScriptOrigin origin(isolate, NewString(isolate, filename));
    std::string content = Loader::GetJsCode(filename); // debugload(filename);
    ScriptCompiler::Source script_source(NewString(isolate, content.c_str()), origin);
    
    Local<String> params[] = {
        NewString(isolate, "No"),
    };
    //     . Value
    // 在 V8 中，Value 是所有 JavaScript 值的基类，包括：
    // 基本值类型：String、Number、Boolean、Undefined、Null。
    // 复杂值类型：Object、Function、Array 等。
    // 因为 Value 是所有 JavaScript 值的基类，所以任何 Local<T> 类型对象（如 Local<Object>、Local<Function> 等）都可以安全地转换为 Local<Value>。
    Local<Value> argv[] = {
        No.As<Value>()
    };
    // 执行 No.js 文件的代码，并传入 No 对象，在 JS 层可以访问 C++ 层导出的功能
    // 编译代码
    Local<Function> func = ScriptCompiler::CompileFunction(context, &script_source, 1, params, 0, nullptr).ToLocalChecked();
    // 执行代码
    func->Call(context, context->Global(), 1, argv).ToLocalChecked();  
    // 执行所有的微任务
    {
        No::MicroTask::MicroTaskScope microTaskScope(env);
    }
    // uv_run 是异步的：
    // 事件循环会检测并处理挂起的异步任务（如网络 I/O、定时器等）。
    // 例如，JavaScript 中的 setTimeout 回调会在事件循环中被执行。
    uv_run(env->loop(), UV_RUN_DEFAULT);
}