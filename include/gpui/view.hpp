#pragma once

#include "element.hpp"
#include "ffi.h"

namespace gpui {

class View {
public:
    virtual ~View() = default;

    virtual Element render() = 0;
};

namespace detail {

inline GpuiNode* view_render_trampoline(GpuiAppCtx* ctx, void* user) {
    CtxGuard guard(ctx);
    return static_cast<View*>(user)->render().release();
}

}
}
