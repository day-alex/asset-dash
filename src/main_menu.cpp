#include "main_menu.h"
#include <ftxui/component/screen_interactive.hpp>        // App
#include <ftxui/component/component.hpp>  // Menu, Renderer
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <ranges>
#include <string>
#include <vector>

using namespace ftxui;

void MainMenu::display() {
  auto dash_accounts = dash_.accounts()
                     | std::views::keys
                     | std::ranges::to<std::vector<std::string>>();

  auto screen = ScreenInteractive::Fullscreen();   // declared before we need its closure

  int left_menu_selected = 0;
  MenuOption menu_option;
  menu_option.on_enter = screen.ExitLoopClosure();

  Component left_menu = Menu(&dash_accounts, &left_menu_selected, menu_option);

  auto renderer = Renderer(left_menu, [&] {
    int account_list_width = Terminal::Size().dimx / 5;

    Elements rows;
    if (!dash_accounts.empty()) {
      const auto& inst = dash_accounts[left_menu_selected];
      for (const auto& acct : dash_.accounts().at(inst)) {
        rows.push_back(hbox({
            text("[" + acct->type_label() + "] "),
            text(acct->name()),
            filler(),                                       // pushes balance right
            text(std::format("${:.2f}", acct->balance())),
        }));
      }
    } else {
      rows.push_back(text("No accounts connected") | dim);
    }

    return hbox({
              left_menu->Render() | size(WIDTH, EQUAL, account_list_width),
              separator(),
              vbox(std::move(rows)) | flex,
          }) |
          border;
  });

  screen.Loop(renderer);
}
