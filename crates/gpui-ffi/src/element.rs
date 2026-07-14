use std::ffi::{c_char, c_int, c_void, CStr};

use gpui::SharedString;

use crate::app::GpuiAppCtx;
use crate::style::{GpuiLength, StyleProps};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct GpuiMouseEvent {
    pub x: f64,
    pub y: f64,
    pub button: c_int,
    pub click_count: c_int,
}

pub type GpuiEventFn = extern "C" fn(*mut c_void, *const GpuiMouseEvent, *mut GpuiAppCtx);
pub type GpuiDropFn = extern "C" fn(*mut c_void);

pub struct Handler {
    pub func: GpuiEventFn,
    pub user: *mut c_void,
    pub drop_fn: Option<GpuiDropFn>,
}

impl Handler {
    pub fn invoke(&self, ev: &GpuiMouseEvent, ctx: *mut GpuiAppCtx) {
        (self.func)(self.user, ev as *const _, ctx);
    }
}

impl Drop for Handler {
    fn drop(&mut self) {
        if let Some(d) = self.drop_fn {
            d(self.user);
        }
    }
}

pub enum NodeKind {
    Div,
    Text(SharedString),
}

pub struct Node {
    pub kind: NodeKind,
    pub style: StyleProps,
    pub children: Vec<Node>,
    pub id: Option<String>,
    pub on_click: Option<Handler>,
    pub on_mouse_down: Option<Handler>,
    pub on_mouse_up: Option<Handler>,
}

impl Node {
    fn new(kind: NodeKind) -> Self {
        Node {
            kind,
            style: StyleProps::default(),
            children: Vec::new(),
            id: None,
            on_click: None,
            on_mouse_down: None,
            on_mouse_up: None,
        }
    }
}

pub(crate) unsafe fn cstr(s: *const c_char) -> Option<String> {
    if s.is_null() {
        None
    } else {
        Some(CStr::from_ptr(s).to_string_lossy().into_owned())
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gpui_node_div() -> *mut Node {
    Box::into_raw(Box::new(Node::new(NodeKind::Div)))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn gpui_node_text(utf8: *const c_char) -> *mut Node {
    let s = if utf8.is_null() {
        SharedString::new_static("")
    } else {
        SharedString::from(CStr::from_ptr(utf8).to_string_lossy())
    };
    Box::into_raw(Box::new(Node::new(NodeKind::Text(s))))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn gpui_node_add_child(parent: *mut Node, child: *mut Node) {
    if child.is_null() {
        return;
    }
    let Some(p) = parent.as_mut() else {
        drop(Box::from_raw(child));
        return;
    };
    p.children.push(*Box::from_raw(child));
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn gpui_node_free(n: *mut Node) {
    if !n.is_null() {
        drop(Box::from_raw(n));
    }
}

macro_rules! node_setters {
    ($(fn $name:ident($($arg:ident: $ty:ty),*) |$nd:ident| $body:block)*) => {$(
        #[unsafe(no_mangle)]
        pub unsafe extern "C" fn $name(n: *mut Node, $($arg: $ty),*) {
            if let Some($nd) = n.as_mut() $body
        }
    )*};
}

node_setters! {
    fn gpui_node_set_id(id: *const c_char) |nd| { nd.id = cstr(id); }

    fn gpui_node_flex(direction: c_int) |nd| {
        nd.style.flex = true;
        nd.style.flex_dir = direction;
    }
    fn gpui_node_flex_grow(grow: f32) |nd| { nd.style.flex_grow = Some(grow); }
    fn gpui_node_flex_wrap(wrap: bool) |nd| { nd.style.flex_wrap = wrap; }
    fn gpui_node_gap(gap_px: f32) |nd| { nd.style.gap = Some(gap_px); }
    fn gpui_node_justify(justify: c_int) |nd| { nd.style.justify = Some(justify); }
    fn gpui_node_items(items: c_int) |nd| { nd.style.items = Some(items); }
    fn gpui_node_w(len: GpuiLength) |nd| { nd.style.w = len.to_gpui(); }
    fn gpui_node_h(len: GpuiLength) |nd| { nd.style.h = len.to_gpui(); }
    fn gpui_node_padding(edge: c_int, v: f32) |nd| { nd.style.padding.set(edge, v); }
    fn gpui_node_margin(edge: c_int, v: f32) |nd| { nd.style.margin.set(edge, v); }
    fn gpui_node_absolute(top: f32, right: f32, bottom: f32, left: f32) |nd| {
        nd.style.absolute = true;
        nd.style.inset_top = (!top.is_nan()).then_some(top);
        nd.style.inset_right = (!right.is_nan()).then_some(right);
        nd.style.inset_bottom = (!bottom.is_nan()).then_some(bottom);
        nd.style.inset_left = (!left.is_nan()).then_some(left);
    }
    fn gpui_node_overflow_hidden(hidden: bool) |nd| { nd.style.overflow_hidden = hidden; }

    fn gpui_node_bg(rgba: u32) |nd| { nd.style.bg = Some(rgba); }
    fn gpui_node_text_color(rgba: u32) |nd| { nd.style.text_color = Some(rgba); }
    fn gpui_node_border(width: f32, rgba: u32) |nd| {
        nd.style.border_width = width;
        nd.style.border_color = Some(rgba);
    }
    fn gpui_node_rounded(radius: f32) |nd| { nd.style.rounded = radius; }
    fn gpui_node_shadow(shadow: c_int) |nd| { nd.style.shadow = shadow; }
    fn gpui_node_opacity(opacity: f32) |nd| { nd.style.opacity = Some(opacity); }

    fn gpui_node_text_size(size_px: f32) |nd| { nd.style.text_size = Some(size_px); }
    fn gpui_node_font_weight(weight: u16) |nd| { nd.style.font_weight = Some(weight); }
    fn gpui_node_font_family(family: *const c_char) |nd| { nd.style.font_family = cstr(family); }
    fn gpui_node_italic(italic: bool) |nd| { nd.style.italic = italic; }

    fn gpui_node_cursor_pointer(enabled: bool) |nd| { nd.style.cursor_pointer = enabled; }
    fn gpui_node_hover_bg(rgba: u32) |nd| { nd.style.hover_bg = Some(rgba); }

    fn gpui_node_on_click(func: GpuiEventFn, user: *mut c_void, drop_fn: Option<GpuiDropFn>) |nd| {
        nd.on_click = Some(Handler { func, user, drop_fn });
    }
    fn gpui_node_on_mouse_down(func: GpuiEventFn, user: *mut c_void, drop_fn: Option<GpuiDropFn>) |nd| {
        nd.on_mouse_down = Some(Handler { func, user, drop_fn });
    }
    fn gpui_node_on_mouse_up(func: GpuiEventFn, user: *mut c_void, drop_fn: Option<GpuiDropFn>) |nd| {
        nd.on_mouse_up = Some(Handler { func, user, drop_fn });
    }
}
