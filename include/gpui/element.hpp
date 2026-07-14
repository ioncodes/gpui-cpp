#pragma once

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "color.hpp"
#include "ffi.h"

namespace gpui {

using MouseEvent = GpuiMouseEvent;

namespace detail {

inline thread_local GpuiAppCtx* g_current_ctx = nullptr;

struct CtxGuard {
    GpuiAppCtx* prev;
    explicit CtxGuard(GpuiAppCtx* ctx) noexcept : prev(g_current_ctx) {
        g_current_ctx = ctx;
    }
    ~CtxGuard() { g_current_ctx = prev; }
    CtxGuard(const CtxGuard&) = delete;
    CtxGuard& operator=(const CtxGuard&) = delete;
};

template <typename Fn>
inline void event_trampoline(void* user, const GpuiMouseEvent* ev,
                             GpuiAppCtx* ctx) {
    CtxGuard guard(ctx);
    (*static_cast<Fn*>(user))(ev ? *ev : MouseEvent{});
}

template <typename Fn>
inline void drop_trampoline(void* user) {
    delete static_cast<Fn*>(user);
}

struct NodeDeleter {
    void operator()(GpuiNode* n) const noexcept { gpui_node_free(n); }
};

}

class Element {
public:
    Element() noexcept = default;
    explicit Element(GpuiNode* node) noexcept : node_(node) {}

    GpuiNode* release() noexcept { return node_.release(); }

    explicit operator bool() const noexcept { return node_ != nullptr; }

    Element child(Element c) && {
        gpui_node_add_child(node_.get(), c.release());
        return std::move(*this);
    }
    Element child(const std::string& text_content) &&;
    Element child(const char* text_content) &&;

    Element id(const std::string& identifier) && {
        gpui_node_set_id(node_.get(), identifier.c_str());
        return std::move(*this);
    }

    Element flex() && { return std::move(*this).flex_row(); }
    Element flex_row() && { gpui_node_flex(node_.get(), GPUI_FLEX_ROW); return std::move(*this); }
    Element flex_col() && { gpui_node_flex(node_.get(), GPUI_FLEX_COL); return std::move(*this); }
    Element flex_grow(float g = 1.0f) && { gpui_node_flex_grow(node_.get(), g); return std::move(*this); }
    Element flex_wrap() && { gpui_node_flex_wrap(node_.get(), true); return std::move(*this); }
    Element gap(float px) && { gpui_node_gap(node_.get(), px); return std::move(*this); }

    Element justify_start() && { gpui_node_justify(node_.get(), GPUI_JUSTIFY_START); return std::move(*this); }
    Element justify_center() && { gpui_node_justify(node_.get(), GPUI_JUSTIFY_CENTER); return std::move(*this); }
    Element justify_end() && { gpui_node_justify(node_.get(), GPUI_JUSTIFY_END); return std::move(*this); }
    Element justify_between() && { gpui_node_justify(node_.get(), GPUI_JUSTIFY_BETWEEN); return std::move(*this); }
    Element justify_around() && { gpui_node_justify(node_.get(), GPUI_JUSTIFY_AROUND); return std::move(*this); }

    Element items_start() && { gpui_node_items(node_.get(), GPUI_ITEMS_START); return std::move(*this); }
    Element items_center() && { gpui_node_items(node_.get(), GPUI_ITEMS_CENTER); return std::move(*this); }
    Element items_end() && { gpui_node_items(node_.get(), GPUI_ITEMS_END); return std::move(*this); }

    Element w(float px) && { gpui_node_w(node_.get(), {GPUI_LEN_PX, px}); return std::move(*this); }
    Element w_pct(float percent) && { gpui_node_w(node_.get(), {GPUI_LEN_PERCENT, percent}); return std::move(*this); }
    Element w_full() && { gpui_node_w(node_.get(), {GPUI_LEN_FULL, 0}); return std::move(*this); }
    Element h(float px) && { gpui_node_h(node_.get(), {GPUI_LEN_PX, px}); return std::move(*this); }
    Element h_pct(float percent) && { gpui_node_h(node_.get(), {GPUI_LEN_PERCENT, percent}); return std::move(*this); }
    Element h_full() && { gpui_node_h(node_.get(), {GPUI_LEN_FULL, 0}); return std::move(*this); }
    Element size(float w_px, float h_px) && {
        gpui_node_w(node_.get(), {GPUI_LEN_PX, w_px});
        gpui_node_h(node_.get(), {GPUI_LEN_PX, h_px});
        return std::move(*this);
    }
    Element size_full() && {
        gpui_node_w(node_.get(), {GPUI_LEN_FULL, 0});
        gpui_node_h(node_.get(), {GPUI_LEN_FULL, 0});
        return std::move(*this);
    }

    Element p(float v) && { gpui_node_padding(node_.get(), GPUI_EDGE_ALL, v); return std::move(*this); }
    Element px(float v) && { gpui_node_padding(node_.get(), GPUI_EDGE_X, v); return std::move(*this); }
    Element py(float v) && { gpui_node_padding(node_.get(), GPUI_EDGE_Y, v); return std::move(*this); }
    Element pt(float v) && { gpui_node_padding(node_.get(), GPUI_EDGE_TOP, v); return std::move(*this); }
    Element pr(float v) && { gpui_node_padding(node_.get(), GPUI_EDGE_RIGHT, v); return std::move(*this); }
    Element pb(float v) && { gpui_node_padding(node_.get(), GPUI_EDGE_BOTTOM, v); return std::move(*this); }
    Element pl(float v) && { gpui_node_padding(node_.get(), GPUI_EDGE_LEFT, v); return std::move(*this); }

    Element m(float v) && { gpui_node_margin(node_.get(), GPUI_EDGE_ALL, v); return std::move(*this); }
    Element mx(float v) && { gpui_node_margin(node_.get(), GPUI_EDGE_X, v); return std::move(*this); }
    Element my(float v) && { gpui_node_margin(node_.get(), GPUI_EDGE_Y, v); return std::move(*this); }
    Element mt(float v) && { gpui_node_margin(node_.get(), GPUI_EDGE_TOP, v); return std::move(*this); }
    Element mr(float v) && { gpui_node_margin(node_.get(), GPUI_EDGE_RIGHT, v); return std::move(*this); }
    Element mb(float v) && { gpui_node_margin(node_.get(), GPUI_EDGE_BOTTOM, v); return std::move(*this); }
    Element ml(float v) && { gpui_node_margin(node_.get(), GPUI_EDGE_LEFT, v); return std::move(*this); }

    Element absolute(float top = NAN, float right = NAN, float bottom = NAN,
                     float left = NAN) && {
        gpui_node_absolute(node_.get(), top, right, bottom, left);
        return std::move(*this);
    }
    Element overflow_hidden() && { gpui_node_overflow_hidden(node_.get(), true); return std::move(*this); }

    Element bg(std::uint32_t rgba) && { gpui_node_bg(node_.get(), rgba); return std::move(*this); }
    Element text_color(std::uint32_t rgba) && { gpui_node_text_color(node_.get(), rgba); return std::move(*this); }
    Element border(float width_px, std::uint32_t rgba) && { gpui_node_border(node_.get(), width_px, rgba); return std::move(*this); }
    Element rounded(float radius_px) && { gpui_node_rounded(node_.get(), radius_px); return std::move(*this); }
    Element shadow_sm() && { gpui_node_shadow(node_.get(), GPUI_SHADOW_SM); return std::move(*this); }
    Element shadow_md() && { gpui_node_shadow(node_.get(), GPUI_SHADOW_MD); return std::move(*this); }
    Element shadow_lg() && { gpui_node_shadow(node_.get(), GPUI_SHADOW_LG); return std::move(*this); }
    Element shadow_xl() && { gpui_node_shadow(node_.get(), GPUI_SHADOW_XL); return std::move(*this); }
    Element opacity(float o) && { gpui_node_opacity(node_.get(), o); return std::move(*this); }

    Element text_size(float px) && { gpui_node_text_size(node_.get(), px); return std::move(*this); }
    Element font_weight(std::uint16_t weight) && { gpui_node_font_weight(node_.get(), weight); return std::move(*this); }
    Element font_bold() && { gpui_node_font_weight(node_.get(), 700); return std::move(*this); }
    Element font_family(const std::string& family) && { gpui_node_font_family(node_.get(), family.c_str()); return std::move(*this); }
    Element italic() && { gpui_node_italic(node_.get(), true); return std::move(*this); }

    Element cursor_pointer() && { gpui_node_cursor_pointer(node_.get(), true); return std::move(*this); }
    Element hover_bg(std::uint32_t rgba) && { gpui_node_hover_bg(node_.get(), rgba); return std::move(*this); }

    template <typename F>
    Element on_click(F fn) && {
        attach(gpui_node_on_click, std::move(fn));
        return std::move(*this);
    }
    template <typename F>
    Element on_mouse_down(F fn) && {
        attach(gpui_node_on_mouse_down, std::move(fn));
        return std::move(*this);
    }
    template <typename F>
    Element on_mouse_up(F fn) && {
        attach(gpui_node_on_mouse_up, std::move(fn));
        return std::move(*this);
    }

private:
    using AttachFn = void (*)(GpuiNode*, GpuiEventFn, void*, GpuiDropFn);

    template <typename F>
    void attach(AttachFn setter, F fn) {
        if constexpr (std::is_invocable_v<F&, const MouseEvent&>) {
            auto* heap = new F(std::move(fn));
            setter(node_.get(), &detail::event_trampoline<F>, heap,
                   &detail::drop_trampoline<F>);
        } else {
            static_assert(std::is_invocable_v<F&>,
                          "handler must be callable as fn(const MouseEvent&) or fn()");
            attach(setter, [f = std::move(fn)](const MouseEvent&) { f(); });
        }
    }

    std::unique_ptr<GpuiNode, detail::NodeDeleter> node_;
};

inline Element div() { return Element(gpui_node_div()); }

inline Element text(const std::string& content) {
    return Element(gpui_node_text(content.c_str()));
}
inline Element text(const char* content) {
    return Element(gpui_node_text(content));
}

inline Element Element::child(const std::string& text_content) && {
    return std::move(*this).child(gpui::text(text_content));
}
inline Element Element::child(const char* text_content) && {
    return std::move(*this).child(gpui::text(text_content));
}

inline void quit() {
    if (detail::g_current_ctx) {
        gpui_app_quit(detail::g_current_ctx);
    }
}

}
