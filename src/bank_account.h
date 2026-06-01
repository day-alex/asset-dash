#pragma once
#include "account.h"

class BankAccount : public Account {
public:
  using Account::Account;
  std::string type_label() const override { return "BNK"; }
};
