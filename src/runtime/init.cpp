#include "runtime/init.hpp"
#include "util/util.hpp"
#include <dlfcn.h>
#include <memory>
#include <cstring>

namespace omega::wass {

auto closeDl = [](void *ptr) { dlclose(ptr);};
using DlHandlePtr = std::unique_ptr<void, decltype(closeDl)>;

constexpr std::string_view START_FUNC_NAME = "_start";

inline static const std::unordered_map<std::string, const char*> lib_alias = {
        { "ld-linux",       LD_SO },
        { "libanl",         LIBANL_SO },
        { "libbrokenlocale",LIBBROKENLOCALE_SO },
        { "libc",           LIBC_SO },
        { "libdl",          LIBDL_SO },
        { "libm",           LIBM_SO },
        { "libnsl",         LIBNSL_SO },
        { "libnss_compat",  LIBNSS_COMPAT_SO },
        { "libnss_dns",     LIBNSS_DNS_SO },
        { "libnss_files",   LIBNSS_FILES_SO },
        { "libnss_hesiod",  LIBNSS_HESIOD_SO },
        { "libnss_ldap",    LIBNSS_LDAP_SO },
        { "libpthread",     LIBPTHREAD_SO },
        { "libresolv",      LIBRESOLV_SO },
        { "librt",          LIBRT_SO },
        { "libthread_db",   LIBTHREAD_DB_SO },
        { "libutil",        LIBUTIL_SO },
};


NativeFuncType getNativeFuncPtr(std::string_view lib, std::string_view sym_name) {
    DlHandlePtr handle(dlopen(lib.data(), RTLD_LAZY));

    if (!handle) {
        throw std::runtime_error("native library not found "  + std::string(dlerror()));
    }
    auto f_ptr = reinterpret_cast<NativeFuncType>(dlsym(handle.get(), sym_name.data()));

    if (!f_ptr) {
        throw std::runtime_error("native symbol not found "  + std::string(sym_name));
    }
    return f_ptr;
}

std::vector<ValType> createNativeFuncParams(std::string_view signature) {
    std::vector<ValType> params;
    for (auto e: signature) {
        switch (e) {
            case native::FuncParams::REF: {
                params.emplace_back(ValType::REF);
                break;
            }
            case native::FuncParams::INT: {
                params.emplace_back(ValType::I64);
                break;
            }
            case native::FuncParams::FLOAT: {
                params.emplace_back(ValType::F64);
                break;
            }
            default: {
                throw std::runtime_error("unknown native func tupe: " + e);
            }
        }
    }
    return params;
}

template <typename BackInserter>
void readImportFuncs(module::WasmModule &module, BackInserter inserter) {
    for (auto &imp : module.importSection) {
        if (imp.kind == module::ImportKind::FUNC) {
            auto native_func_sig  = util::parse_call(imp.name);
            std::string_view libname;
            auto it = lib_alias.find(imp.module);
            if (it != lib_alias.end()) {
                libname = it->second;
            } else {
                libname = imp.module;
            }
            RuntimeFunction runtimeFunction;
            runtimeFunction.isNative = true;
            runtimeFunction.native_ptr = getNativeFuncPtr(libname, native_func_sig.first);
            runtimeFunction.signature.params = createNativeFuncParams(native_func_sig.second);
            if (runtimeFunction.signature.params.size() != module.typesSection.at(imp.typeIndex).params.size()) {
                throw std::runtime_error("not matching native func signatures: " + native_func_sig.first);
            }
            runtimeFunction.signature.results = module.typesSection.at(imp.typeIndex).results;
            *inserter = runtimeFunction;
            ++inserter;
        }
    }
}

template <typename BackInserter>
void readWasmFunction(module::WasmModule &module, BackInserter inserter) {
    auto func_ind_section = module.functionSection;
    i32 index = 0;
    for (auto &body : module.codeSection) {
        RuntimeFunction runtimeFunction;
        for (auto localVar : body.locals) {
            std::fill_n(std::back_inserter(runtimeFunction.locals), localVar.count, Operand(localVar.type, 0L));
        }
        runtimeFunction.signature = module.typesSection.at(func_ind_section.at(index).ind);
        runtimeFunction.labelMap = createLabelMap(body.code);
        runtimeFunction.code = std::move(body.code);

        *inserter = std::move(runtimeFunction);
        ++inserter;
        ++index;
    }
}

u32 findStartFuncInd(module::WasmModule &module) {
    for (auto &exp : module.exportSection) {
        if (exp.kind == module::ExportKind::FUNC_EXP) {
            if (exp.name == START_FUNC_NAME) {
                return exp.index;
            }
        }
    }
    throw std::runtime_error("_start function not found");
}

LabelMap createLabelMap(const std::vector<u8> &code) {
    LabelMap map;
    std::stack<ControlBlock*> control_stack;

    u32 size = code.size();
    for (u32 i = 0; i < size; ++i) {
        u8 op = code[i];
        switch (op) {
            case runtime::Bytecode::block:
            case runtime::Bytecode::loop:
            case runtime::Bytecode::if_: {
                u32 label_ind = i + 1;
                u8 block_type = code.at(label_ind);
                if (
                        block_type != ValType::BLOCK &&
                        block_type != ValType::F32 &&
                        block_type != ValType::F64 &&
                        block_type != ValType::I32 &&
                        block_type != ValType::I32
                        ) {
                    continue;
                }
                ControlBlock block{.type = static_cast<runtime::Bytecode>(op), .start = (i + 2), .end = 0};
                map.insert({label_ind, block});
                control_stack.push(&map.find(label_ind)->second);
                break;
            }
            case runtime::Bytecode::end:
            case runtime::Bytecode::else_:{
                if (control_stack.empty()) {
                    if (i != size - 1) {
                        continue;
                    }
                } else {
                    auto block = control_stack.top();
                    block->end = i;
                    control_stack.pop();
                }
            }
        }
    }
    return map;
}

std::vector<RuntimeFunction> initRuntimeFunctions(module::WasmModule &module) {
    std::vector<RuntimeFunction> funcs;
    readImportFuncs(module, std::back_inserter(funcs));
    readWasmFunction(module, std::back_inserter(funcs));
    return funcs;
}

GlobalsContainer initGlobals(module::WasmModule &module) {
    GlobalsContainer globals;
    for (auto &g : module.globalSection) {
        GlobalVar globalVar;
        globalVar.mut = g.mutable_;
        globalVar.op.type = g.valType;

        if (g.valType == ValType::I32 || g.valType == ValType::I64) {
            globalVar.op.val.i = util::readLEB128(g.initExpr.data() + 1);
        } else {
            std::memcpy(&globalVar.op.val.f, g.initExpr.data() + 1, sizeof(F64));
        }

        globals.emplace_back(globalVar);
    }
    return globals;
}

MemsContainer initMemory(module::WasmModule &module) {
    MemsContainer mems;
    for (auto lim : module.memorySection) {
        mems.emplace_back(lim.min * WASM_PAGE_SIZE);
    }
    return mems;
}

void initData(module::WasmModule &module, MemsContainer &mems) {
    for (auto &d : module.dataSection) {
        u32 ind = d.memIndex;
        u32 off = util::readLEB128(d.offsetExpr.data() + 1);
        std::memcpy(mems.at(ind).data() + off, d.data.data(), d.data.size());
    }
}
}