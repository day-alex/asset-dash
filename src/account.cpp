#include "account.h"
#include <format>
#include <iostream>

Account::Account(const json& acct_info, const std::string& inst)
  : account_id_(acct_info["account_id"]),
    name_(acct_info["name"]),
    institution_(inst),
    balance_(acct_info["balances"]["current"].get<double>()) {
  if (acct_info.contains("subtype") && !acct_info["subtype"].is_null()) {
    subtype_ = acct_info["subtype"].get<std::string>();
  }
}

std::string Account::summary_str() const {
  return std::format("\t{:<28} ${:>12.2f}\n", name_, balance_);
}
