#include "runtime_structures.hpp"
#include "wasm_types.hpp"
#include <dlfcn.h>
#include <gnu/lib-names.h>
#include <cstring>

namespace omega::wass {



void Store::initHeap(WasmModule &module) {
    if (module.memories) {
        //todo hardcoded 0 for now
        auto min = module.memories.value().memories[0].min;
        heap.resize(min * WASM_PAGE_SIZE);
    }
}

void Store::initFunc(WasmModule &module) {
    if (module.imports) {
        for (auto imp : module.imports.value().imports) {
            if (imp.kind == ImportKind::FUNC && imp.module == "native") {
                void * handle = dlopen (LIBC_SO, RTLD_LAZY);
                if (handle == nullptr) {
                    printf("its over\n");
                    abort();
                }
                auto prf = reinterpret_cast<NativeFuncType>(dlsym(handle, imp.name.data()));

                RuntimeFunction function;
                function.isNative = true;
                function.signature = &module.types.value().types[imp.typeIndex];
                function.ctx.native_ptr = prf;
                func_.emplace_back(function);
                dlclose(handle);
            }
        }
    }

    int32_t sz = module.functions.typeIndices.size();
    for (int32_t i = 0; i < sz; ++i) {
        auto t_ind = module.functions.typeIndices[i];
        RuntimeFunction function;
        function.isNative = false;
        function.signature = &module.types.value().types[t_ind];

        function.ctx.code = &module.code.value().bodies[i];

        func_.emplace_back(function);
    }
}

void Store::init(WasmModule &module) {
    initFunc(module);
    initHeap(module);
    initDataSection(module);
}

std::vector<RuntimeFunction> &Store::getFunc() {
    return func_;
}

char *Store::getHeap() {
    return heap.data();
}


uint32_t readOffsetExpr(std::vector<u8> &v) {
    uint32_t result = 0;
    uint8_t shift = 0;

    for (auto it = v.begin() + 1, end = v.end(); it != end; ++it) {
        uint8_t byte = *it;
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
    }
    return result;
}

void Store::initDataSection(WasmModule &module) {
    auto data_section = module.data;
    if (data_section) {
        for (auto segment : data_section.value().segments ) {
            uint32_t offset = readOffsetExpr(segment.offsetExpr);
            std::memcpy(getHeap() + offset, segment.data.data(), segment.data.size());
        }
    }
}


} // namespace omega::wass