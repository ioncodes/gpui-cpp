use std::ffi::{c_char, c_void};

use gpui::{px, size, App, AppContext as _, Bounds, SharedString, TitlebarOptions, WindowBounds,
    WindowOptions};

use crate::element::{cstr, GpuiDropFn};
use crate::view::{CppView, GpuiRenderFn};

#[repr(C)]
pub struct GpuiAppCtx {
    pub(crate) app: *mut App,
}

impl GpuiAppCtx {
    pub(crate) fn for_app(app: &mut App) -> GpuiAppCtx {
        GpuiAppCtx { app: app as *mut App }
    }
}

pub type GpuiReadyFn = extern "C" fn(*mut GpuiAppCtx, *mut c_void);

#[repr(C)]
pub struct GpuiWindowDesc {
    pub title: *const c_char,
    pub width: f64,
    pub height: f64,
}

#[unsafe(no_mangle)]
pub extern "C" fn gpui_app_run(on_ready: GpuiReadyFn, user_data: *mut c_void) {
    gpui_platform::application().run(move |cx: &mut App| {
        let mut ctx = GpuiAppCtx::for_app(cx);
        on_ready(&mut ctx as *mut _, user_data);
    });
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn gpui_open_window(
    ctx: *mut GpuiAppCtx,
    desc: *const GpuiWindowDesc,
    render: GpuiRenderFn,
    user: *mut c_void,
    user_drop: Option<GpuiDropFn>,
) -> bool {
    let Some(ctx) = ctx.as_mut() else {
        return false;
    };
    let app = &mut *ctx.app;
    let d = &*desc;

    let title: Option<SharedString> = cstr(d.title).map(SharedString::from);

    let bounds = Bounds::centered(None, size(px(d.width as f32), px(d.height as f32)), app);
    let options = WindowOptions {
        window_bounds: Some(WindowBounds::Windowed(bounds)),
        titlebar: Some(TitlebarOptions {
            title,
            ..Default::default()
        }),
        ..Default::default()
    };

    let ok = app
        .open_window(options, |_window, cx| {
            cx.new(|_| CppView {
                render_fn: render,
                user_data: user,
                drop_fn: user_drop,
            })
        })
        .is_ok();

    if ok {
        app.activate(true);
    }
    ok
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn gpui_app_quit(ctx: *mut GpuiAppCtx) {
    if let Some(ctx) = ctx.as_mut() {
        (*ctx.app).quit();
    }
}
