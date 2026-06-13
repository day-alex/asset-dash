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

  std::vector<std::string> dash_accounts{"Summary"};
  dash_accounts.append_range(dash_.accounts() | std::views::keys);

  auto screen = ScreenInteractive::Fullscreen();   // declared before we need its closure

  int left_menu_selected = 0;
  MenuOption menu_option;
  menu_option.on_enter = screen.ExitLoopClosure();

  Component left_menu = Menu(&dash_accounts, &left_menu_selected, menu_option);

  auto renderer = Renderer(left_menu, [&] {
    int account_list_width = Terminal::Size().dimx / 5;

    Elements rows;
    if (left_menu_selected == 0) {
        std::string summary = dash_.summary_view_str();
        Elements lines;
        std::stringstream ss(summary);   // or split manually
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(text(line));
        }
        rows.push_back(vbox(std::move(lines)));
    }
    else {
      const auto& inst = dash_accounts[left_menu_selected];

      for (const auto& acct : dash_.accounts().at(inst)) {
        rows.push_back(hbox({
            text("[" + acct->type_label() + "] "),
            text(acct->name()),
            filler(),                                       // pushes balance right
            text(std::format("${:.2f}", acct->balance())),
        }));
      }
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
