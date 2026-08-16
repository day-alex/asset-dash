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

Account::Account(std::string account_id, std::string name, std::string institution, double balance)
  : account_id_(std::move(account_id)),
    name_(std::move(name)),
    institution_(std::move(institution)),
    balance_(balance) {
}

std::string Account::summary_str() const {
  // TODO: dynamic formatting based on longest institution name
  return std::format("\t{:<40} ${:>12.2f}\n", institution_, balance_);
}
