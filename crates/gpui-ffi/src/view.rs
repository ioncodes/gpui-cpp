use std::ffi::c_void;

use gpui::{
    div, prelude::*, AnyElement, ClickEvent, Context, ElementId, MouseButton, MouseDownEvent,
    MouseUpEvent, Pixels, Point, SharedString, Window,
};

use crate::app::GpuiAppCtx;
use crate::element::{GpuiDropFn, GpuiMouseEvent, Handler, Node, NodeKind};
use crate::style::apply;

pub type GpuiRenderFn = extern "C" fn(*mut GpuiAppCtx, *mut c_void) -> *mut Node;

pub struct CppView {
    pub render_fn: GpuiRenderFn,
    pub user_data: *mut c_void,
    pub drop_fn: Option<GpuiDropFn>,
}

impl Drop for CppView {
    fn drop(&mut self) {
        if let Some(d) = self.drop_fn {
            d(self.user_data);
        }
    }
}

impl Render for CppView {
    fn render(&mut self, _window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let mut fctx = GpuiAppCtx::for_app(cx);
        let ptr = (self.render_fn)(&mut fctx as *mut _, self.user_data);
        if ptr.is_null() {
            return div().into_any_element();
        }
        let mut next_auto_id = 0u64;
        build(unsafe { *Box::from_raw(ptr) }, cx, &mut next_auto_id)
    }
}

fn dispatch(handler: &Handler, ev: GpuiMouseEvent, cx: &mut Context<CppView>) {
    let mut fctx = GpuiAppCtx::for_app(cx);
    handler.invoke(&ev, &mut fctx as *mut _);
    cx.notify();
}

fn button_code(b: MouseButton) -> i32 {
    match b {
        MouseButton::Left => 0,
        MouseButton::Right => 1,
        MouseButton::Middle => 2,
        _ => 3,
    }
}

fn mouse_event(position: Point<Pixels>, button: MouseButton, click_count: usize) -> GpuiMouseEvent {
    GpuiMouseEvent {
        x: f64::from(position.x),
        y: f64::from(position.y),
        button: button_code(button),
        click_count: click_count as i32,
    }
}

macro_rules! mouse_listener {
    ($el:ident, $cx:ident, $name:ident: $ev:ty) => {
        if let Some(h) = $name {
            $el = $el.$name(
                MouseButton::Left,
                $cx.listener(move |_this, ev: &$ev, _w, cx| {
                    dispatch(&h, mouse_event(ev.position, ev.button, ev.click_count), cx);
                }),
            );
        }
    };
}

fn build(node: Node, cx: &mut Context<CppView>, next_auto_id: &mut u64) -> AnyElement {
    let Node {
        kind,
        mut style,
        children,
        id,
        on_click,
        on_mouse_down,
        on_mouse_up,
    } = node;

    match kind {
        NodeKind::Text(s) => apply(div(), &mut style).child(s).into_any_element(),

        NodeKind::Div => {
            let mut el = apply(div(), &mut style);

            if style.cursor_pointer {
                el = el.cursor_pointer();
            }
            if let Some(hb) = style.hover_bg {
                el = el.hover(move |s| s.bg(gpui::rgba(hb)));
            }

            for c in children {
                el = el.child(build(c, cx, next_auto_id));
            }

            mouse_listener!(el, cx, on_mouse_down: MouseDownEvent);
            mouse_listener!(el, cx, on_mouse_up: MouseUpEvent);

            let needs_state = on_click.is_some() || id.is_some();
            if needs_state {
                let eid = match id {
                    Some(s) => ElementId::from(SharedString::from(s)),
                    None => {
                        let v = *next_auto_id;
                        *next_auto_id += 1;
                        ElementId::NamedInteger(SharedString::new_static("gpui_cpp_auto"), v)
                    }
                };

                let mut stateful = el.id(eid);
                if let Some(h) = on_click {
                    stateful = stateful.on_click(cx.listener(
                        move |_this, _ev: &ClickEvent, _w, cx| {
                            let e = GpuiMouseEvent {
                                x: 0.0,
                                y: 0.0,
                                button: 0,
                                click_count: 1,
                            };
                            dispatch(&h, e, cx);
                        },
                    ));
                }
                
                stateful.into_any_element()
            } else {
                el.into_any_element()
            }
        }
    }
}
