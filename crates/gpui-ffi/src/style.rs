use gpui::{px, relative, rgba, Div, FontWeight, Length, SharedString, Styled};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct GpuiLength {
    pub kind: u8,
    pub value: f32,
}

impl GpuiLength {
    pub fn to_gpui(self) -> Option<Length> {
        match self.kind {
            0 => Some(px(self.value).into()),
            1 => Some(relative(self.value / 100.0).into()),
            2 => Some(relative(1.0).into()),
            _ => None,
        }
    }
}

#[derive(Clone, Copy, Debug, Default)]
pub struct Edges {
    pub top: Option<f32>,
    pub right: Option<f32>,
    pub bottom: Option<f32>,
    pub left: Option<f32>,
}

impl Edges {
    pub fn set(&mut self, which: i32, v: f32) {
        match which {
            0 => {
                self.top = Some(v);
                self.right = Some(v);
                self.bottom = Some(v);
                self.left = Some(v);
            }
            1 => {
                self.left = Some(v);
                self.right = Some(v);
            }
            2 => {
                self.top = Some(v);
                self.bottom = Some(v);
            }
            3 => self.top = Some(v),
            4 => self.right = Some(v),
            5 => self.bottom = Some(v),
            6 => self.left = Some(v),
            _ => {}
        }
    }
}

#[derive(Clone, Debug, Default)]
pub struct StyleProps {
    pub flex: bool,
    pub flex_dir: i32,
    pub flex_grow: Option<f32>,
    pub flex_wrap: bool,
    pub gap: Option<f32>,
    pub justify: Option<i32>,
    pub items: Option<i32>,
    pub w: Option<Length>,
    pub h: Option<Length>,
    pub padding: Edges,
    pub margin: Edges,
    pub absolute: bool,
    pub inset_top: Option<f32>,
    pub inset_right: Option<f32>,
    pub inset_bottom: Option<f32>,
    pub inset_left: Option<f32>,
    pub overflow_hidden: bool,

    pub bg: Option<u32>,
    pub text_color: Option<u32>,
    pub border_width: f32,
    pub border_color: Option<u32>,
    pub rounded: f32,
    pub shadow: i32,
    pub opacity: Option<f32>,

    pub text_size: Option<f32>,
    pub font_weight: Option<u16>,
    pub font_family: Option<String>,
    pub italic: bool,

    pub cursor_pointer: bool,
    pub hover_bg: Option<u32>,
}

macro_rules! edges_px {
    ($el:ident: $($opt:expr => $m:ident),* $(,)?) => {$(
        if let Some(v) = $opt {
            $el = $el.$m(px(v));
        }
    )*};
}

pub fn apply(mut el: Div, s: &mut StyleProps) -> Div {
    if s.flex {
        el = el.flex();
        el = if s.flex_dir == 1 { el.flex_col() } else { el.flex_row() };
    }
    if let Some(g) = s.flex_grow {
        el = el.flex_grow(g);
    }
    if s.flex_wrap {
        el = el.flex_wrap();
    }
    if let Some(g) = s.gap {
        el = el.gap(px(g));
    }
    if let Some(j) = s.justify {
        el = match j {
            0 => el.justify_start(),
            1 => el.justify_center(),
            2 => el.justify_end(),
            3 => el.justify_between(),
            _ => el.justify_around(),
        };
    }
    if let Some(i) = s.items {
        el = match i {
            0 => el.items_start(),
            1 => el.items_center(),
            _ => el.items_end(),
        };
    }
    if let Some(w) = s.w {
        el = el.w(w);
    }
    if let Some(h) = s.h {
        el = el.h(h);
    }

    edges_px!(el:
        s.padding.top => pt, s.padding.right => pr,
        s.padding.bottom => pb, s.padding.left => pl,
        s.margin.top => mt, s.margin.right => mr,
        s.margin.bottom => mb, s.margin.left => ml,
    );

    if s.absolute {
        el = el.absolute();
        edges_px!(el:
            s.inset_top => top, s.inset_right => right,
            s.inset_bottom => bottom, s.inset_left => left,
        );
    }
    if s.overflow_hidden {
        el = el.overflow_hidden();
    }

    if let Some(c) = s.bg {
        el = el.bg(rgba(c));
    }
    if let Some(c) = s.text_color {
        el = el.text_color(rgba(c));
    }
    if s.border_width > 0.0 {
        el = el.border(px(s.border_width));
        if let Some(c) = s.border_color {
            el = el.border_color(rgba(c));
        }
    }
    if s.rounded > 0.0 {
        el = el.rounded(px(s.rounded));
    }
    el = match s.shadow {
        1 => el.shadow_sm(),
        2 => el.shadow_md(),
        3 => el.shadow_lg(),
        4 => el.shadow_xl(),
        _ => el,
    };
    if let Some(o) = s.opacity {
        el = el.opacity(o);
    }

    if let Some(ts) = s.text_size {
        el = el.text_size(px(ts));
    }
    if let Some(w) = s.font_weight {
        el = el.font_weight(FontWeight(w as f32));
    }
    if let Some(f) = s.font_family.take() {
        el = el.font_family(SharedString::from(f));
    }
    if s.italic {
        el = el.italic();
    }

    el
}
