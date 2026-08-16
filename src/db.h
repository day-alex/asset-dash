#pragma once

#include "account.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <vector>
#include <string>
#include <tuple>


class DB {
public:
    explicit DB(const std::string& path);

    void init();
    int upsert(const Account& acct);

    std::vector<std::string> get_access_tokens();

    void snapshot_balance(int balance_id, double balance);
    std::vector<std::pair<std::string, double>> get_history(int account_id, int days = 30);
    std::vector<std::pair<std::string, double>> get_portfolio_history(int days = 30);

    // account_id, name, type, balance
    using CustomAccountRow = std::tuple<int, std::string, std::string, double>;
    int insert_custom_account(const std::string& name, const std::string& type, double balance);
    std::vector<CustomAccountRow> get_custom_accounts();

private:
    SQLite::Database db_;
};
