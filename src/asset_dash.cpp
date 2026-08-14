#include "asset_dash.h"
#include "account_factory.h"
#include "plaid_linker.h"
#include <httplib.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <format>

namespace {
    std::string db_path() {
        auto config_dir = std::filesystem::path(std::getenv("HOME")) / ".config" / "asset_dash";
        std::filesystem::create_directories(config_dir);
        return (config_dir / "asset_dash.db").string();
    }
}

AssetDash::AssetDash() : db_(db_path()) {
    db_.init();
    migrate_legacy_tokens();
    refresh();
}

void AssetDash::refresh() {
    accounts_.clear();
    load_tokens_and_fetch();
}

std::pair<std::string, std::vector<std::unique_ptr<Account>>>
AssetDash::fetch_and_store_token(const std::string& access_token) {
    httplib::SSLClient cli("production.plaid.com");

    json body = {
        {"client_id", env_.client_id()},
        {"secret", env_.secret()},
        {"access_token", access_token}
    };
    auto res = cli.Post("/accounts/get", body.dump(), "application/json");
    if (!res || res->status != 200) {
      std::cerr << "Plaid call failed for token: " << access_token.substr(0, 10) << "...\n";
      if (res) std::cerr << "Status: " << res->status << " Body: " << res->body << "\n";
      return {};
    }

    auto data = json::parse(res->body);
    std::string inst = data["item"]["institution_name"].get<std::string>();

    std::vector<std::unique_ptr<Account>> accounts;
    for (const auto& acct : data["accounts"]) {
        auto account = make_account(acct, inst);
        account->set_access_token(access_token);
        int id = db_.upsert(*account);
        db_.snapshot_balance(id, account->balance());
        accounts.push_back(std::move(account));
    }
    return {inst, std::move(accounts)};
}

void AssetDash::load_tokens_and_fetch() {
    for (const auto& access_token : db_.get_access_tokens()) {
        auto [inst, accounts] = fetch_and_store_token(access_token);
        if (inst.empty()) continue;
        for (auto& account : accounts) accounts_[inst].push_back(std::move(account));
    }
}

// One-time migration from the legacy tokens.json file to the SQLite-backed
// account store. Safe to call on every startup: it no-ops once tokens.json
// has been renamed aside.
void AssetDash::migrate_legacy_tokens() {
    auto config_dir = std::filesystem::path(std::getenv("HOME")) / ".config" / "asset_dash";
    auto legacy_path = config_dir / "tokens.json";
    if (!std::filesystem::exists(legacy_path)) return;

    std::ifstream f(legacy_path);
    json tokens = json::parse(f);
    f.close();

    for (const auto& t : tokens) {
        fetch_and_store_token(t["access_token"].get<std::string>());
    }

    std::filesystem::rename(legacy_path, config_dir / "tokens.json.migrated");
    std::cout << "Migrated " << tokens.size() << " legacy access token(s) into the database.\n";
}

void AssetDash::link_new_account() {
    PlaidLinker linker(env_.client_id(), env_.secret(), db_);
    linker.link_account();
    // maybe return added account institution name?
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

std::string AssetDash::summary_view_str() const {
  std::string summary{};
  for (const auto& [inst, accts] : accounts_) {
    for (const auto& acct : accts) {
      summary += acct->summary_str();
    }
  }

  return summary;
}


std::vector<float> AssetDash::portfolio_graph_values() {
  PortfolioValues pv = db_.get_portfolio_history();

  std::vector<float> vals{};

  for (const auto& pair : pv) {
    vals.push_back(pair.second);
  }

  return vals;
}
