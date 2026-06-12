#pragma once
#include <vector>
#include "account.h"
#include "plaid_env.h"

class AssetDash {
public:
  AssetDash();
  void refresh();   // pulls balances
  void run_menu();  // loop

  double net_worth() const;
  std::string print_all();
  const std::unordered_map<
              std::string, std::vector<std::unique_ptr<Account>>>& 
              accounts() const { return accounts_; }

  void link_new_account();
  void debug_ascensus();

private:
  PlaidEnv env_;
  std::unordered_map<std::string, std::vector<std::unique_ptr<Account>>> accounts_;

  void load_tokens_and_fetch();
  void print_by_type();
};

