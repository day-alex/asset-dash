#include "main_menu.h"
#include <ftxui/component/screen_interactive.hpp>        // App
#include <ftxui/component/component.hpp>  // Menu, Renderer
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <ranges>
#include <string>
#include <vector>
#include <functional>
#include <thread>

using namespace ftxui;

namespace {
    struct LeftEntry {
        std::string label;
        std::function<void()> on_enter;
        std::function<Element()> render_right;
    };
}

void MainMenu::display() {
    auto screen = ScreenInteractive::Fullscreen();
    int selected = 0;

    std::vector<LeftEntry> entries;
    std::vector<std::string> option_labels;

    std::function<void()> rebuild_entries;
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
                return vbox({text("Summary") | bold | hcenter,
                    separator(),
                    vbox(std::move(lines))
                });
            },
        });

        bool refreshing = false;
        std::string refresh_status;

        entries.push_back({
            "Refresh",
            [&] {
                if (refreshing) return;
                refreshing = true;
                refresh_status = "Refreshing...";
                std::thread([&] {
                    dash_.refresh();
                    screen.Post([&] {
                        refreshing = false;
                        refresh_status = "Account balances refreshed.";
                        rebuild_entries();
                    });
                    screen.PostEvent(Event::Custom);
                }).detach();
            },
            [&] {
                return text(refreshing ? refresh_status : "Press Enter to refresh account balances.")
                        | hcenter;
            },
        });

        bool linking = false;
        std::string link_status;

        entries.push_back({
            "Link New Account",
            [&] {
                if (linking) return;
                linking = true;
                link_status = "Opening browser... complete the Plaid flow there";

                std::thread([&] {
                    dash_.link_new_account();
                    screen.Post([&] {
                        linking = false;
                        link_status = "Account successfully linked";
                        rebuild_entries(); // new acct will appear
                    });
                    screen.PostEvent(Event::Custom);
                }).detach();
            },
            [&] {
                return vbox({
                    text("Link New Account") | bold | hcenter,
                    separator(),
                    text(linking ? link_status : "Press Enter to link a new account")
                        | hcenter,
                });
            },
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
    };

    rebuild_entries();

    MenuOption option;
    option.on_enter = [&] { entries[selected].on_enter(); };
    Component left_menu = Menu(&option_labels, &selected, option);

    auto renderer = Renderer(left_menu, [&] {
        return vbox({
            text("Asset Dash") | bold | hcenter,
            hbox({
                left_menu->Render()
                    | size(WIDTH, EQUAL, Terminal::Size().dimx / 5),
                separator(),
                entries[selected].render_right() | flex,
            }) | border,
        });
    });

    screen.Loop(renderer);
}