#include "main_menu.h"
#include "value_graph.h"
#include <ftxui/component/screen_interactive.hpp>        // App
#include <ftxui/component/component.hpp>  // Menu, Renderer
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <ranges>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <exception>

using namespace ftxui;

namespace {
    struct LeftEntry {
        std::string label;
        std::function<void()> on_enter;
        std::function<Element()> render_right;
        Component form_component = nullptr;  // set only for entries needing keyboard input focus
    };
}

void MainMenu::display() const {
    auto screen = ScreenInteractive::Fullscreen();
    int selected = 0;
    
    ValueGraph vg{dash_.portfolio_graph_values()};
    auto refresh_graph = [&] { vg = ValueGraph{dash_.portfolio_graph_values()}; };
    std::vector<LeftEntry> entries;
    std::vector<std::string> option_labels;

    // --- Add Custom Account form (state persists across menu rebuilds) ---
    auto custom_name = std::make_shared<std::string>();
    auto custom_balance = std::make_shared<std::string>();
    auto custom_status = std::make_shared<std::string>();
    auto custom_types = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"Bank", "Investment", "Retirement", "Other"});
    auto custom_type_selected = std::make_shared<int>(0);

    Component custom_name_input = Input(custom_name.get(), "Account name");
    Component custom_balance_input = Input(custom_balance.get(), "Balance, e.g. 12345.67");
    Component custom_type_radio = Radiobox(custom_types.get(), custom_type_selected.get());

    std::function<void()> rebuild_entries;

    Component custom_save_button = Button("Save Account", [&, custom_name, custom_balance,
                                            custom_status, custom_types, custom_type_selected] {
        if (custom_name->empty()) {
            *custom_status = "Enter an account name.";
            return;
        }
        double balance;
        try {
            balance = std::stod(*custom_balance);
        } catch (const std::exception&) {
            *custom_status = "Enter a valid balance, e.g. 12345.67";
            return;
        }
        dash_.add_custom_account(*custom_name, (*custom_types)[*custom_type_selected], balance);
        *custom_status = "Added " + *custom_name + ".";
        custom_name->clear();
        custom_balance->clear();
        refresh_graph();
        rebuild_entries();
    });

    Component custom_form = Container::Vertical({
        custom_name_input,
        custom_balance_input,
        custom_type_radio,
        custom_save_button,
    });

    Component right_tab = Container::Tab({}, &selected);
    rebuild_entries = [&]() {
        entries.clear();
        entries.push_back({
            "Summary",
            [] {},
            [&] {
                Elements lines;
                std::stringstream ss(dash_.summary_view_str());
                std::string line;
                while (std::getline(ss, line)) lines.push_back(text(line));
                const int dimx = Terminal::Size().dimx;
                const int menu_w = dimx / 5;
                const int panel_w = dimx - menu_w - 3;   // -1 menu separator, -2 border
                const int half = (panel_w - 1) / 2;      // -1 for the inner separator

                return vbox({
                    text("Summary") | bold | hcenter,
                    separator(),
                    hbox({
                        vbox(std::move(lines)) | size(WIDTH, EQUAL, half),
                        separator(),
                        vbox({
                            text(std::format("${:.2f}", dash_.net_worth()))
                                | bold | color(Color::Green) | hcenter,
                            separator(),
                            graph(std::ref(vg)) | color(Color::GreenLight) | flex,
                        }) | flex,                                  // <-- claims the panel width
                    }) | flex,
                });
            },
        });

        auto refreshing = std::make_shared<bool>(false);
        auto refresh_status = std::make_shared<std::string>();

        entries.push_back({
            "Refresh",
            [&, refreshing, refresh_status] {
                if (*refreshing) return;
                *refreshing = true;
                *refresh_status = "Refreshing...";
                std::thread([&, refreshing, refresh_status] {
                    dash_.refresh();
                    screen.Post([&, refreshing, refresh_status] {
                        *refreshing = false;
                        *refresh_status = "Account balances refreshed.";
                        refresh_graph();
                        rebuild_entries();
                    });
                    screen.PostEvent(Event::Custom);
                }).detach();
            },
            [&, refreshing, refresh_status] {
                return text(*refreshing ? *refresh_status : "Press Enter to refresh account balances.")
                        | hcenter;
            },
        });

        auto linking = std::make_shared<bool>(false);
        auto link_status = std::make_shared<std::string>();

        entries.push_back({
            "Link New Account",
            [&, linking, link_status] {
                if (*linking) return;
                *linking = true;
                *link_status = "Opening browser... complete the Plaid flow there";

                std::thread([&, linking, link_status] {
                    dash_.link_new_account();
                    screen.Post([&] {
                        *linking = false;
                        *link_status = "Account successfully linked";
                        refresh_graph();
                        rebuild_entries(); // new acct will appear
                    });
                    screen.PostEvent(Event::Custom);
                }).detach();
            },
            [&, linking, link_status] {
                return vbox({
                    text("Link New Account") | bold | hcenter,
                    separator(),
                    text(*linking ? *link_status : "Press Enter to link a new account")
                        | hcenter,
                });
            },
        });

        entries.push_back({
            "Add Custom Account",
            [&] {
                *custom_status = "";
                custom_form->TakeFocus();
            },
            [&] {
                return vbox({
                    text("Add Custom Account") | bold | hcenter,
                    separator(),
                    hbox({text("Name:    "), custom_name_input->Render()}),
                    hbox({text("Balance: "), custom_balance_input->Render()}),
                    text("Type:"),
                    custom_type_radio->Render(),
                    separator(),
                    custom_save_button->Render(),
                    text(*custom_status) | color(Color::GreenLight),
                });
            },
            custom_form,
        });

        for (const auto &inst: dash_.accounts() | std::views::keys) {
            entries.push_back({
                inst,
                [&, inst] {}, // this will go into account specific things later
                [&, inst] {
                    Elements rows;
                    for (const auto& acct : dash_.accounts().at(inst)) {
                        rows.push_back(hbox({
                            text("[" + acct->type_label() + "] "),
                            text(acct->name()),
                            filler(),
                            text(std::format("${:.2f}", acct->balance())),
                        }));
                    }

                    return vbox(std::move(rows));
                },
            });
        }

        option_labels.clear();
        for (auto& e : entries) option_labels.push_back(e.label);
        selected = std::clamp(selected, 0, static_cast<int>(entries.size()) - 1);

        // Rebuild the (event-routing only) tab container to match entries; visual
        // rendering still goes through each entry's render_right() below.
        right_tab->DetachAllChildren();
        for (auto& e : entries) {
            if (e.form_component) {
                right_tab->Add(e.form_component);
            } else {
                right_tab->Add(Renderer([render_right = e.render_right] { return render_right(); }));
            }
        }
    };

    rebuild_entries();

    MenuOption option;
    option.on_enter = [&] { entries[selected].on_enter(); };
    Component left_menu = Menu(&option_labels, &selected, option);

    Component root = Container::Horizontal({left_menu, right_tab});

    const auto renderer = Renderer(root, [&] {
        return vbox({
            text("Asset Dash") | bold | hcenter,
            hbox({
                left_menu->Render() | size(WIDTH, EQUAL, Terminal::Size().dimx / 5),
                separator(),
                entries[selected].render_right() | flex,
            }) | border | flex,                          // <-- fills the terminal height
        });
    });

    screen.Loop(renderer);
}
