#![allow(clippy::missing_safety_doc)]

pub mod app;
pub mod element;
pub mod style;
pub mod view;

use std::ffi::c_char;

pub const GPUI_FFI_ABI_LEVEL: u32 = 1;

#[unsafe(no_mangle)]
pub extern "C" fn gpui_ffi_version() -> *const c_char {
    concat!(env!("CARGO_PKG_VERSION"), "\0").as_ptr() as *const c_char
}

#[unsafe(no_mangle)]
pub extern "C" fn gpui_ffi_abi_level() -> u32 {
    GPUI_FFI_ABI_LEVEL
}
