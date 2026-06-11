#pragma once

#include "asset_dash.h"

class MainMenu {
  public:
    explicit MainMenu(AssetDash& dash) : dash_(dash) {}
    void display();
    
  private:
    AssetDash& dash_;
};

