//
// Created by Alex Day on 8/8/26.
//

#include "db.h"


DB::DB(const std::string& path)
    : db_(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
        db_.exec("PRAGMA journal_mode=WAL");
        db_.exec("PRAGMA foreign_keys=ON");
}

void DB::init() {

}

int DB::upsert(const Account& acct) {
    return 0;
}

std::vector<Account> DB::list_accounts() const {

}

void DB::snapshot_balance(int balance_id, double balance) {

}

std::vector<std::pair<std::string, double>> DB::get_history(int account_id) {

}

std::vector<std::pair<std::string, double>> DB::get_portfolio_history() {

}