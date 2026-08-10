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
            id                  INTEGER PRIMARY KEY AUTOINCREMENT,
            plaid_account_id    TEXT NOT NULL UNIQUE,
            access_token        TEXT NOT NULL,
            name                TEXT NOT NULL,
            type                TEXT,
            created_at          TEXT DEFAULT (datetime('now'))
        )
    )");

    db_.exec(R"(
        CREATE TABLE IF NOT EXISTS balance_history (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            account_id      INTEGER NOT NULL,
            balance         REAL NOT NULL,
            snapshot_date   TEXT NOT NULL DEFAULT (date('now')),
            FOREIGN KEY (account_id) REFERENCES accounts(id),
            UNIQUE(account_id, snapshot_date)
        )
    )");
}

int DB::upsert(const Account& acct) {
    SQLite::Statement stmt(db_, R"(
        INSERT INTO accounts (plaid_account_id, access_token, name, type)
        VALUES (?, ?, ?, ?)
        ON CONFLICT(plaid_account_id) DO UPDATE SET
            access_token=excluded.access_token,
            name=excluded.name,
            type=excluded.type
        RETURNING id
    )");
    stmt.bind(1, acct.account_id());
    stmt.bind(2, acct.access_token());
    stmt.bind(3, acct.name());
    stmt.bind(4, acct.type_label());
    stmt.executeStep();
    return stmt.getColumn(0).getInt();
}

std::vector<std::string> DB::get_access_tokens() {
    SQLite::Statement stmt(db_, "SELECT DISTINCT access_token FROM accounts");

    std::vector<std::string> tokens;
    while (stmt.executeStep()) {
        tokens.push_back(stmt.getColumn(0).getString());
    }
    return tokens;
}

void DB::snapshot_balance(int account_id, double balance) {
    SQLite::Statement stmt(db_, R"(
        INSERT INTO balance_history (account_id, balance)
        VALUES (?, ?)
        ON CONFLICT(account_id, snapshot_date)
        DO UPDATE SET balance=excluded.balance
    )");
    stmt.bind(1, account_id);
    stmt.bind(2, balance);
    stmt.exec();
}

std::vector<std::pair<std::string, double>> DB::get_history(int account_id, int days) {
    SQLite::Statement stmt(db_, R"(
        SELECT snapshot_date, balance
        FROM balance_history
        WHERE account_id = ?
        ORDER BY snapshot_date DESC
        LIMIT ?
    )");
    stmt.bind(1, account_id);
    stmt.bind(2, days);

    std::vector<std::pair<std::string, double>> history;
    while (stmt.executeStep()) {
        history.emplace_back(
            stmt.getColumn(0).getString(),
            stmt.getColumn(1).getDouble()
        );
    }
    return history;
}

std::vector<std::pair<std::string, double>> DB::get_portfolio_history(int days) {
    SQLite::Statement stmt(db_, R"(
        SELECT snapshot_date, SUM(balance)
        FROM balance_history
        GROUP BY snapshot_date
        ORDER BY snapshot_date DESC
        LIMIT ?
    )");
    stmt.bind(1, days);

    std::vector<std::pair<std::string, double>> history;
    while (stmt.executeStep()) {
        history.emplace_back(
            stmt.getColumn(0).getString(),
            stmt.getColumn(1).getDouble()
        );
    }
    return history;
}