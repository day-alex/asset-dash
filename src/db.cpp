//
// Created by Alex Day on 8/8/26.
//

#include "db.h"
#include <chrono>


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

    // Custom (non-Plaid) accounts share the accounts/balance_history tables;
    // is_custom distinguishes them since they have no real plaid_account_id/access_token.
    bool has_is_custom = false;
    SQLite::Statement columns(db_, "PRAGMA table_info(accounts)");
    while (columns.executeStep()) {
        if (columns.getColumn(1).getString() == "is_custom") has_is_custom = true;
    }
    if (!has_is_custom) {
        db_.exec("ALTER TABLE accounts ADD COLUMN is_custom INTEGER NOT NULL DEFAULT 0");
    }
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

int DB::insert_custom_account(const std::string& name, const std::string& type, double balance) {
    // No Plaid identity exists for a custom account; synthesize a unique
    // placeholder to satisfy the plaid_account_id UNIQUE/NOT NULL constraint.
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::string synthetic_id = "custom-" + std::to_string(now);

    SQLite::Statement stmt(db_, R"(
        INSERT INTO accounts (plaid_account_id, access_token, name, type, is_custom)
        VALUES (?, '', ?, ?, 1)
        RETURNING id
    )");
    stmt.bind(1, synthetic_id);
    stmt.bind(2, name);
    stmt.bind(3, type);
    stmt.executeStep();
    int id = stmt.getColumn(0).getInt();

    snapshot_balance(id, balance);
    return id;
}

std::vector<DB::CustomAccountRow> DB::get_custom_accounts() {
    SQLite::Statement stmt(db_, R"(
        SELECT a.id, a.name, a.type, bh.balance
        FROM accounts a
        JOIN balance_history bh ON bh.account_id = a.id
        WHERE a.is_custom = 1
          AND bh.snapshot_date = (
              SELECT MAX(snapshot_date) FROM balance_history WHERE account_id = a.id
          )
    )");

    std::vector<CustomAccountRow> rows;
    while (stmt.executeStep()) {
        rows.emplace_back(
            stmt.getColumn(0).getInt(),
            stmt.getColumn(1).getString(),
            stmt.getColumn(2).getString(),
            stmt.getColumn(3).getDouble()
        );
    }
    return rows;
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
