#include "asset_dash.h"
#include "account_factory.h"
#include "plaid_linker.h"
#include <httplib.h>
#include <fstream>
#include <iostream>
#include <format>

AssetDash::AssetDash() {
    refresh();
}

void AssetDash::refresh() {
    accounts_.clear();
    load_tokens_and_fetch();
}

void AssetDash::load_tokens_and_fetch() {
    std::ifstream f(std::string(std::getenv("HOME")) + "/.config/asset_dash/tokens.json");
    auto tokens = json::parse(f);

    httplib::SSLClient cli("production.plaid.com");

    for (const auto& t : tokens) {
        json body = {
            {"client_id", env_.client_id()},
            {"secret", env_.secret()},
            {"access_token", t["access_token"]}
        };
        auto res = cli.Post("/accounts/get", body.dump(), "application/json");
        if (!res || res->status != 200) continue;

        auto data = json::parse(res->body);
        // std::cout << data.dump(2) << "\n";

        std::string inst = data["item"]["institution_name"].get<std::string>();

        for (const auto& acct : data["accounts"]) {
            accounts_[inst].push_back(make_account(acct, inst));
        }
    }
}

void AssetDash::link_new_account() {
    PlaidLinker linker(env_.client_id(), env_.secret());
    linker.link_account();
    // maybe return added account institution name?
}

void AssetDash::debug_ascensus() {
  json body = {
    {"client_id", env_.client_id()},
    {"secret", env_.secret()},
    {"institution_id", "ins_116972"},
    {"country_codes", {"us"}},
    {"options", {
        {"include_optional_metadata", true},
        {"include_status", true}
    }}
  };

  httplib::SSLClient cli("production.plaid.com");
  auto res = cli.Post("/institutions/get_by_id", body.dump(), "application/json");
  if (!res) {
    std::cerr << "request failed (no response from plaid)\n";
    return;
  }
  if (res->status != 200) {
    std::cerr << "http " << res->status << "\n" << res->body << "\n";
    return;
  }

  auto parsed = json::parse(res->body);
  std::cout << parsed.dump(2) << std::endl;
}

double AssetDash::net_worth() const {
    double total = 0.0;
    for (const auto &accts: accounts_ | std::views::values) {
        for (const auto& acct : accts) {
          // credit balances should subtract; refine later
          if (acct->type_label() == "cc ") total -= acct->balance();
          else total += acct->balance();
        }
    }
    return total;
}

std::string AssetDash::print_all() {
    // for (const auto& [inst, accts] : accounts_) {
    //   std::cout << "\n>>>>>> " << inst << "<<<<<<\n";
    //   for (const auto& acct : accts) {
    //     // acct->print_summary();
    //   }
    // }
    // return std::format("\nnet worth: ${:.2f}\n", net_worth());
  return "";
}

std::string AssetDash::summary_view_str() const {
  std::string summary{};
  for (const auto& [inst, accts] : accounts_) {
    for (const auto& acct : accts) {
      summary += acct->summary_str();
    }
  }

  return summary;
}

void AssetDash::run_menu() {
    while (true) {
        std::cout << "\n=== asset_dash ===\n"
                  << "1. View all accounts\n"
                  << "2. Refresh balances\n"
                  << "3. Link New Account\n"
                  << "d. Debug ascensus\n"
                  << "q. Quit\n> ";
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1") print_all();
        else if (choice == "2") { refresh(); std::cout << "Refreshed.\n"; }
        else if (choice == "3") link_new_account();
        else if (choice == "d") debug_ascensus();
        else if (choice == "q") break;
    }
}
