#pragma once
#include "account.h"
#include <string>

// Manually-entered account with no Plaid connection (e.g. a 401k).
class CustomAccount : public Account {
public:
    CustomAccount(std::string account_id, std::string name, std::string type, double balance);

    std::string type_label() const override { return "CST"; }
};
