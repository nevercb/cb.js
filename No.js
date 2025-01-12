const {
    loader,
    process,
    console,
} = No.buildin;
// No.js 文件中首先会获取刚才 C++ 层注册的各种底层能力，这里非常重要的一个地方，通过 V8 的能力在 C++ 层把 C++ 模块导出的功能引入了 JS 领域，从此 JS 层就强大了起来。其中最重要的是 loader 模块的功能，因为 No.js 需要初始化内置的各种 JS 模块，也就是 loaderNativeModule 函数的逻辑，这个函数会根据模块配置，加载所有内置 JS 模块的代码并执行，然后在 No.libs 对象中记录各个 JS 模块导出的功能，
function loaderNativeModule() {
    const modules = [
        {
            module: 'libs/uv/index.js',
            name: 'uv',
        },
        {
            module: 'libs/os/index.js',
            name: 'os',
        },
        {
            module: 'libs/console/index.js',
            name: 'console',
            after: (exports) => {
                global.console = exports;
            }
        },
        {
            module: 'libs/vm/index.js',
            name: 'vm',
        },
        {
            module: 'libs/module/index.js',
            name: 'module'
        },
        {
            module: 'libs/events/index.js',
            name: 'events'
        },
        {
            module: 'libs/timer/index.js',
            name: 'timer',
            after: (exports) => {
                global.setTimeout = exports.setTimeout;
                global.setInterval = exports.setInterval;
            }
        },
        {
            module: 'libs/process/index.js',
            name: 'process',
            after: (exports) => {
                global.process = exports;
            }
        },
        {
            module: 'libs/buffer/index.js',
            name: 'buffer',
            after: (exports) => {
                global.Buffer = exports;
            }
        },
        {
            module: 'libs/nextTick/index.js',
            name: 'microtask',
            after: (exports) => {
                global.process.nextTick = exports.nextTick;
                global.process.enqueueMicrotask = exports.enqueueMicrotask;
            }
        },
        {
            module: 'libs/immediate/index.js',
            name: 'immediate',
            after: (exports) => {
                global.process.setImmediate = exports.setImmediate;
            }
        },
        {
            module: 'libs/dns/index.js',
            name: 'dns'
        },
        {
            module: 'libs/pipe/index.js',
            name: 'pipe'
        },
        {
            module: 'libs/udp/index.js',
            name: 'udp'
        },
        {
            module: 'libs/fs/index.js',
            name: 'fs'
        },
        {
            module: 'libs/tcp/index.js',
            name: 'tcp'
        },
        {
            module: 'libs/http/index.js',
            name: 'http'
        },
        {
            module: 'libs/worker/index.js',
            name: 'worker'
        },
        {
            module: 'libs/child_process/index.js',
            name: 'child_process',
        },
        {
            module: 'libs/cluster/index.js',
            name: 'cluster',
        },
        {
            module: 'libs/signal/index.js',
            name: 'signal',
        },
    ];
    No.libs = {};
    for (let i = 0; i < modules.length; i++) {
        const module = {
            exports: {},
        };
        loader[process.env.Local ? 'compile' : 'compileNative'](modules[i].module).call(null, loader.compile, module.exports, module, No);
        No.libs[modules[i].name] = module.exports;
        typeof modules[i].after === 'function' && modules[i].after(module.exports);
    }
}

loaderNativeModule();

if (process.isMainThread) {
    No.libs.module.load(process.argv[1]);
} else {
    // load并执行用户层代码
    No.libs.module.load("libs/worker/main.js");
    No.libs.module.load(process.argv[1]);
}
