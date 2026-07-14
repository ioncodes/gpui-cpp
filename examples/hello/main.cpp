#include <gpui/gpui.hpp>

#include <functional>
#include <memory>
#include <string>

namespace {

constexpr auto kBase = gpui::rgb(0x1e1e2e);
constexpr auto kSurface = gpui::rgb(0x313244);
constexpr auto kText = gpui::rgb(0xcdd6f4);
constexpr auto kSubtext = gpui::rgb(0xa6adc8);
constexpr auto kBlue = gpui::rgb(0x89b4fa);
constexpr auto kLavender = gpui::rgb(0xb4befe);
constexpr auto kRed = gpui::rgb(0xf38ba8);
constexpr auto kCrust = gpui::rgb(0x11111b);

gpui::Element button(const std::string& label, std::uint32_t color,
                     std::function<void()> on_click) {
    return gpui::div()
        .px(20)
        .py(10)
        .rounded(8)
        .bg(color)
        .hover_bg(kLavender)
        .cursor_pointer()
        .shadow_md()
        .child(gpui::text(label).text_color(kCrust).font_weight(600))
        .on_click(std::move(on_click));
}

class Counter : public gpui::View {
public:
    gpui::Element render() override {
        return gpui::div()
            .flex_col()
            .items_center()
            .justify_center()
            .size_full()
            .gap(20)
            .bg(kBase)
            .child(gpui::text("gpui-cpp").text_size(14).text_color(kSubtext))
            .child(gpui::text("Count: " + std::to_string(count_))
                       .text_size(36)
                       .font_bold()
                       .text_color(kText))
            .child(gpui::div()
                       .flex_row()
                       .gap(12)
                       .child(button("-1", kBlue, [this] { --count_; }))
                       .child(button("+1", kBlue, [this] { ++count_; }))
                       .child(button("Quit", kRed, [] { gpui::quit(); })));
    }

private:
    int count_ = 0;
};

}

int main() {
    gpui::App app;
    app.run([](gpui::AppContext& cx) {
        cx.open_window(gpui::WindowOptions{}.title("Counter").size(480, 320),
                       std::make_unique<Counter>());
    });
    return 0;
}
