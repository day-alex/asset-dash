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
    db_.exec(R"(
        CREATE TABLE IF NOT EXISTS accounts (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            plaid_id        TEXT NOT NULL UNIQUE,
            access_token    TEXT NOT NULL,
            name            TEXT NOT NULL,
            type            TEXT,
            created_at TEXT DEFAULT (datetime('now'))
        )
    )");

    db_.exec(R"(
        CREATE TABLE IF NOT EXISTS balance_history (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            account_id    INTEGER NOT NULL,
            balance       REAL NOT NULL,
            snapshot_date TEXT NOT NULL DEFAULT (date('now')),
            FOREIGN KEY (account_id) REFERENCES accounts(id),
            UNIQUE(account_id, snapshot_date)
        )
    )");
}

int DB::upsert(const Account& acct) {
    SQLite::Statement stmt(db_, R"(
        INSERT INTO accounts (plaid_id, name, type)
        VALUES (?, ?, ?)
        ON CONFLICT(plaid_id) DO UPDATE SET name=excluded.name, type=excluded.type
        RETURNING id
    )");
    stmt.bind(1, acct.plaid_id);
    stmt.bind(2, acct.name);
    stmt.bind(3, acct.type);
    stmt.executeStep();
    return stmt.getColumn(0).getInt();
}

std::vector<Account> DB::list_accounts() const {

}

void DB::snapshot_balance(int balance_id, double balance) {

}

std::vector<std::pair<std::string, double>> DB::get_history(int account_id) {

}

std::vector<std::pair<std::string, double>> DB::get_portfolio_history() {

}