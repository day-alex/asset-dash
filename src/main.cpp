#include "asset_dash.h"
#include "main_menu.h"

int main() {
    AssetDash dash;
    MainMenu main_menu(dash);
    main_menu.display();

    return 0;
}
