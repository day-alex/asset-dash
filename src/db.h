#pragma once

#include "account.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <vector>
#include <string>


class DB {
public:
    explicit DB(const std::string& path);

    void init();
    int upsert(const Account& acct);

    void snapshot_balance(int balance_id, double balance);
    std::vector<std::pair<std::string, double>> get_history(int account_id, int days = 30);
    std::vector<std::pair<std::string, double>> get_portfolio_history(int days = 30);

private:
    SQLite::Database db_;
};
