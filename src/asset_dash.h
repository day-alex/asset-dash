#pragma once
#include <vector>
#include "account.h"
#include "db.h"
#include "plaid_env.h"

class AssetDash {

public:
  using PortfolioValues = std::vector<std::pair<std::string, double>>;
  
  AssetDash();
  void refresh();   // pulls balances

  double net_worth() const;
  std::string summary_view_str() const;
  const std::unordered_map<
              std::string, std::vector<std::unique_ptr<Account>>>& 
              accounts() const { return accounts_; }

  void link_new_account();
  std::vector<float> portfolio_graph_values();
private:
  PlaidEnv env_;
  DB db_;
  std::unordered_map<std::string, std::vector<std::unique_ptr<Account>>> accounts_;

  void load_tokens_and_fetch();
  void migrate_legacy_tokens();
  std::pair<std::string, std::vector<std::unique_ptr<Account>>>
      fetch_and_store_token(const std::string& access_token);
};

