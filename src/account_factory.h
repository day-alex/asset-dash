#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include "account.h"

std::unique_ptr<Account> make_account(const nlohmann::json& acct_info, const std::string& inst);
