#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "element.hpp"
#include "ffi.h"
#include "view.hpp"

namespace gpui {

class WindowOptions {
public:
    WindowOptions& title(std::string t) {
        title_ = std::move(t);
        return *this;
    }
    WindowOptions& size(double width, double height) {
        width_ = width;
        height_ = height;
        return *this;
    }

private:
    friend class AppContext;
    std::string title_;
    double width_ = 800.0;
    double height_ = 600.0;
};

class AppContext {
public:
    explicit AppContext(GpuiAppCtx* raw) noexcept : raw_(raw) {}

    bool open_window(const WindowOptions& options, std::unique_ptr<View> view) {
        GpuiWindowDesc desc{
            options.title_.empty() ? nullptr : options.title_.c_str(),
            options.width_,
            options.height_,
        };
        View* raw_view = view.release();
        bool ok = gpui_open_window(raw_, &desc,
                                   &detail::view_render_trampoline, raw_view,
                                   &detail::drop_trampoline<View>);
        if (!ok) {
            delete raw_view;
        }
        return ok;
    }

    void quit() { gpui_app_quit(raw_); }

    GpuiAppCtx* raw() noexcept { return raw_; }

private:
    GpuiAppCtx* raw_;
};

class App {
public:
    App() {
        assert(gpui_ffi_abi_level() == GPUI_FFI_ABI_LEVEL &&
               "gpui-cpp: header/staticlib ABI level mismatch");
    }

    template <typename F>
    void run(F on_ready) {
        static_assert(std::is_invocable_v<F&, AppContext&>,
                      "on_ready must be callable as fn(AppContext&)");
        gpui_app_run(
            [](GpuiAppCtx* ctx, void* user) {
                detail::CtxGuard guard(ctx);
                AppContext app_ctx(ctx);
                (*static_cast<F*>(user))(app_ctx);
            },
            &on_ready);
    }
};

}
