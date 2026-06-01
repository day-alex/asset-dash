#pragma once
#include "account.h"

class InvestmentAccount : public Account {
public:
  using Account::Account;
  std::string type_label() const override { return "INV"; }
  // we adding investment related stuff here later
  // posititions, port diversity, etc
};
