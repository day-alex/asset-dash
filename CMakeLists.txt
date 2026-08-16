cmake_minimum_required(VERSION 3.20)
project(asset_dash LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
        GIT_TAG v7.0.0
)
FetchContent_Declare(httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.18.3
)
FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_Declare(
        SQLiteCpp
        GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
        GIT_TAG        3.3.2
)
FetchContent_MakeAvailable(ftxui httplib json SQLiteCpp)

target_compile_options(SQLiteCpp PUBLIC -include cstdint)

find_package(OpenSSL REQUIRED)

add_executable(asset_dash
        src/main.cpp
        src/plaid_linker.cpp
        src/plaid_env.cpp
        src/asset_dash.cpp
        src/account.cpp
        src/account_factory.cpp
        src/custom_account.cpp
        src/main_menu.cpp
        src/value_graph.cpp
        src/db.cpp
)

target_link_libraries(asset_dash PRIVATE
        ftxui::screen
        ftxui::dom
        ftxui::component
        httplib::httplib
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        SQLiteCpp
)

target_compile_options(asset_dash PRIVATE -fsanitize=address -g)
target_link_options(asset_dash PRIVATE -fsanitize=address)


