#include "main_menu.h"
#include <ftxui/component/component.hpp>          // Menu, Renderer, Container
#include <ftxui/component/screen_interactive.hpp> // ScreenInteractive
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <string>
#include <vector>

using namespace ftxui;

void MainMenu::display() {
  std::vector<std::string> dash_accounts;
  for (const auto& [inst, accts] : dash_.accounts()) {
    dash_accounts.push_back(inst);
  }

  auto screen = ScreenInteractive::Fullscreen();  // or TerminalOutput()

  int left_menu_selected = 0;
  MenuOption menu_option;
  menu_option.on_enter = screen.ExitLoopClosure();

  Component left_menu = Menu(&dash_accounts, &left_menu_selected, menu_option);

  auto renderer = Renderer(left_menu, [&] {
    int account_list_width = Terminal::Size().dimx / 5;
    return hbox({
               left_menu->Render() | size(WIDTH, EQUAL, account_list_width),
               separator(),
               vbox({
                   text("Selected: " + dash_accounts[left_menu_selected]),
                   // account detail for dash_.accounts() goes here
               }) | flex,
           }) |
           border;
  });

  screen.Loop(renderer);
}
