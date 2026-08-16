#include "custom_account.h"

CustomAccount::CustomAccount(std::string account_id, std::string name, std::string type, double balance)
  : Account(std::move(account_id), std::move(name), "Custom", balance) {
    subtype_ = std::move(type);
}
