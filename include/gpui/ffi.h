#ifndef GPUI_FFI_H
#define GPUI_FFI_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GpuiAppCtx GpuiAppCtx;
typedef struct GpuiNode GpuiNode;

typedef enum GpuiFlexDirection {
    GPUI_FLEX_ROW = 0,
    GPUI_FLEX_COL = 1
} GpuiFlexDirection;

typedef enum GpuiJustify {
    GPUI_JUSTIFY_START = 0,
    GPUI_JUSTIFY_CENTER = 1,
    GPUI_JUSTIFY_END = 2,
    GPUI_JUSTIFY_BETWEEN = 3,
    GPUI_JUSTIFY_AROUND = 4
} GpuiJustify;

typedef enum GpuiAlign {
    GPUI_ITEMS_START = 0,
    GPUI_ITEMS_CENTER = 1,
    GPUI_ITEMS_END = 2
} GpuiAlign;

typedef enum GpuiLengthKind {
    GPUI_LEN_PX = 0,
    GPUI_LEN_PERCENT = 1,
    GPUI_LEN_FULL = 2,
    GPUI_LEN_AUTO = 3
} GpuiLengthKind;

typedef struct GpuiLength {
    uint8_t kind;
    float value;
} GpuiLength;

typedef enum GpuiEdge {
    GPUI_EDGE_ALL = 0,
    GPUI_EDGE_X = 1,
    GPUI_EDGE_Y = 2,
    GPUI_EDGE_TOP = 3,
    GPUI_EDGE_RIGHT = 4,
    GPUI_EDGE_BOTTOM = 5,
    GPUI_EDGE_LEFT = 6
} GpuiEdge;

typedef enum GpuiShadow {
    GPUI_SHADOW_NONE = 0,
    GPUI_SHADOW_SM = 1,
    GPUI_SHADOW_MD = 2,
    GPUI_SHADOW_LG = 3,
    GPUI_SHADOW_XL = 4
} GpuiShadow;

typedef struct GpuiMouseEvent {
    double x;
    double y;
    int button;
    int click_count;
} GpuiMouseEvent;

typedef struct GpuiWindowDesc {
    const char* title;
    double width;
    double height;
} GpuiWindowDesc;

typedef void      (*GpuiReadyFn)(GpuiAppCtx* ctx, void* user_data);
typedef GpuiNode* (*GpuiRenderFn)(GpuiAppCtx* ctx, void* user_data);
typedef void      (*GpuiDropFn)(void* user_data);
typedef void      (*GpuiEventFn)(void* user_data, const GpuiMouseEvent* event,
                                 GpuiAppCtx* ctx);

const char* gpui_ffi_version(void);
uint32_t    gpui_ffi_abi_level(void);

#define GPUI_FFI_ABI_LEVEL 1u

void gpui_app_run(GpuiReadyFn on_ready, void* user_data);
void gpui_app_quit(GpuiAppCtx* ctx);

bool gpui_open_window(GpuiAppCtx* ctx, const GpuiWindowDesc* desc,
                      GpuiRenderFn render, void* user_data,
                      GpuiDropFn user_drop);

GpuiNode* gpui_node_div(void);
GpuiNode* gpui_node_text(const char* utf8);
void      gpui_node_add_child(GpuiNode* parent, GpuiNode* child);
void      gpui_node_free(GpuiNode* node);
void      gpui_node_set_id(GpuiNode* node, const char* id);

void gpui_node_flex(GpuiNode* node, int direction);
void gpui_node_flex_grow(GpuiNode* node, float grow);
void gpui_node_flex_wrap(GpuiNode* node, bool wrap);
void gpui_node_gap(GpuiNode* node, float gap_px);
void gpui_node_justify(GpuiNode* node, int justify);
void gpui_node_items(GpuiNode* node, int items);
void gpui_node_w(GpuiNode* node, GpuiLength len);
void gpui_node_h(GpuiNode* node, GpuiLength len);
void gpui_node_padding(GpuiNode* node, int edge, float px);
void gpui_node_margin(GpuiNode* node, int edge, float px);
void gpui_node_absolute(GpuiNode* node, float top, float right, float bottom,
                        float left);
void gpui_node_overflow_hidden(GpuiNode* node, bool hidden);

void gpui_node_bg(GpuiNode* node, uint32_t rgba);
void gpui_node_text_color(GpuiNode* node, uint32_t rgba);
void gpui_node_border(GpuiNode* node, float width_px, uint32_t rgba);
void gpui_node_rounded(GpuiNode* node, float radius_px);
void gpui_node_shadow(GpuiNode* node, int shadow);
void gpui_node_opacity(GpuiNode* node, float opacity);

void gpui_node_text_size(GpuiNode* node, float size_px);
void gpui_node_font_weight(GpuiNode* node, uint16_t weight);
void gpui_node_font_family(GpuiNode* node, const char* family);
void gpui_node_italic(GpuiNode* node, bool italic);

void gpui_node_cursor_pointer(GpuiNode* node, bool enabled);
void gpui_node_hover_bg(GpuiNode* node, uint32_t rgba);
void gpui_node_on_click(GpuiNode* node, GpuiEventFn fn, void* user_data,
                        GpuiDropFn drop_fn);
void gpui_node_on_mouse_down(GpuiNode* node, GpuiEventFn fn, void* user_data,
                             GpuiDropFn drop_fn);
void gpui_node_on_mouse_up(GpuiNode* node, GpuiEventFn fn, void* user_data,
                           GpuiDropFn drop_fn);

#ifdef __cplusplus
}
#endif

#endif
