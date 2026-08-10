#include "plaid_linker.h"
#include <cstdlib>
#include <iostream>

using json = nlohmann::json;

PlaidLinker::PlaidLinker(const std::string& client_id, const std::string& secret, DB& db)
    : client_id_(client_id),
      secret_(secret),
      db_(db)   {}

std::string PlaidLinker::create_link_token() {
  json body{
    {"client_id", client_id_},
    {"secret", secret_},
    {"user", {{"client_user_id", "alex"}}},
    {"client_name", "Asset Dash"},
    {"products", {"assets"}},
    {"country_codes", {"US"}},
    {"language", "en"},
    {"hosted_link", {
      {"completion_redirect_uri", "http://localhost:8080/callback"}
    }}
  };

  auto res = cli_.Post("/link/token/create", body.dump(), "application/json");
  if (!res || res->status != 200) {
      std::cerr << "Failed to create link token\n";
      if (res) std::cerr << res->body << "\n";
      exit(1);
  }

  auto parsed = json::parse(res->body);
  link_token_ = parsed["link_token"].get<std::string>();

  return parsed["hosted_link_url"].get<std::string>();
}


void PlaidLinker::wait_for_callback() {
    httplib::Server svr;

    svr.Get("/callback", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content("<h1>Connected! You can close this tab.</h1>", "text/html");
        svr.stop();
    });

    // std::cout << "Waiting for bank login...\n";
    svr.listen("localhost", 8080);
}

std::string PlaidLinker::get_public_token() {
    json body = {
        {"client_id", client_id_},
        {"secret", secret_},
        {"link_token", link_token_}
    };

    auto res = cli_.Post("/link/token/get", body.dump(), "application/json");
    if (!res || res->status != 200) {
        std::cerr << "Failed to get link session\n";
        if (res) std::cerr << res->body << "\n";
        exit(1);
    }
    
    auto parsed = json::parse(res->body);
    auto& results = parsed["link_sessions"][0]["results"]["item_add_results"];
    if (!results.empty()) {
      return results[0]["public_token"].get<std::string>();
    }

    std::cerr << "No public_token in repsonse." << std::endl;
    return "";
}

void PlaidLinker::exchange_public_token(const std::string& public_token) {
    json body = {
        {"client_id", client_id_},
        {"secret", secret_},
        {"public_token", public_token}
    };

    auto res = cli_.Post("/item/public_token/exchange", body.dump(), "application/json");
    if (!res || res->status != 200) {
        std::cerr << "Failed to exchange token\n";
        if (res) std::cerr << res->body << "\n";
        exit(1);
    }

    auto parsed = json::parse(res->body);
    save_to_db(parsed["access_token"].get<std::string>());
}

void PlaidLinker::save_to_db(const std::string& access_token) {
    json body = {
        {"client_id", client_id_},
        {"secret", secret_},
        {"access_token", access_token}
    };

    auto res = cli_.Post("/accounts/get", body.dump(), "application/json");
    if (!res || res->status != 200) {
        std::cerr << "Failed to fetch accounts\n";
        if (res) std::cerr << res->body << "\n";
        return;
    }

    auto data = json::parse(res->body);
    std::string inst = data["item"]["institution_name"].get<std::string>();

    for (const auto& acct : data["accounts"]) {
        auto account = make_account(acct, inst);
        account->set_access_token(access_token);
        int id = db_.upsert(*account);
        db_.snapshot_balance(id, account->balance());
    }
}

void PlaidLinker::link_account() {
    auto url = create_link_token();
    // std::cout << "Opening browser...\n";
    // std::system(("xdg-open '" + url + "'").c_str());
    std::system(("open '" + url + "'").c_str());
    wait_for_callback();
    auto public_token = get_public_token();
    if (public_token.empty()) {
        std::cerr << "No public token received\n";
        return;
    }

    exchange_public_token(public_token);
    // std::cout << "Successfully linked account!\n";
}
