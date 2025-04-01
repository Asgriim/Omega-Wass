#include "wasm_vm.hpp"
#include "bytecode.hpp"
#include "wasm_module_parser.hpp"
#include <iostream>

namespace omega::wass {

Frame WassVM::createFrame(int32_t func_ind) {
    Frame frame;
    auto f_ptr = &ctx_.store.getFunc()[func_ind];
    frame.func_inst = f_ptr;
    if (!f_ptr->isNative) {
        frame.code = &f_ptr->ctx.code->code;
    }

    if (!f_ptr->ctx.code->locals.empty()) {
        auto sz = f_ptr->ctx.code->locals[0].count;
        frame.locals.resize(sz);
    }

    return frame;
}

void WassVM::start() {
    auto start_frame = createFrame(1);
    ctx_.frameStack.push(start_frame);
    ControlBlock block;
    block.start_ip = 0;
    block.end_ip = start_frame.code->size()  - 1;
    ctx_.blockStack.push(block);
    execute();
}

void WassVM::execute() {
    while (!ctx_.frameStack.empty() && !ctx_.blockStack.empty()) {
        Frame &frame = ctx_.frameStack.top();
        auto ip = frame.ip;
        auto op = static_cast<WasmOpcode>(frame.code->at(ip));
        ++frame.ip;
        dispatch_table[op](frame, &ctx_);
    }
}

void WassVM::initFromFile(std::string_view path) {
    module_=  ModuleParser::parseFromFile(path);
    ctx_.store.init(module_);

}

}