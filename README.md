# Asset Dash

A terminal UI for tracking net worth across financial accounts. It links
bank and investment accounts through Plaid and also supports manually
entered accounts (401k, pensions, or anything else without a Plaid
connection). Balances are snapshotted daily to a local SQLite database so
you can see net worth trend over time.

## Features

- Link accounts via Plaid's hosted Link flow (opens in your browser).
- Add custom, non-Plaid accounts by hand (name, balance, type).
- Daily balance history stored locally in SQLite.
- Summary view with total net worth and a graph of net worth over time;
  hover the graph with your mouse to see the value on a given day.
- Per-institution / per-account breakdown in the left-hand menu.

## Requirements

- CMake 3.20+
- A C++23 compiler (the project builds with AppleClang / Clang or GCC)
- OpenSSL (development headers), found via `find_package(OpenSSL REQUIRED)`
- Internet access on first build to fetch dependencies (FTXUI, cpp-httplib,
  nlohmann/json, SQLiteCpp) via CMake `FetchContent`
- A [Plaid](https://plaid.com/) developer account and API credentials, if
  you want to link real bank/investment accounts. Custom accounts do not
  require Plaid.

On macOS, OpenSSL can be installed with:

```
brew install openssl
```

## Building

```
cmake -S . -B build
cmake --build build
```

The resulting binary is `build/asset_dash`.

## Configuration

Plaid credentials are read from an env file at:

```
~/.config/asset_dash/.env
```

Create that file with your Plaid production client ID and secret:

```
PLAID_CLIENT_ID=your_client_id
PLAID_PROD_SECRET=your_secret
```

This step is only required if you plan to use "Link New Account". Adding
custom accounts works without any Plaid configuration.

The SQLite database (`asset_dash.db`) is created automatically in the same
`~/.config/asset_dash/` directory on first run.

## Running

```
./build/asset_dash
```

This launches a full-screen terminal UI. Use the arrow keys to navigate the
left-hand menu and Enter to select an item:

- **Summary** — total net worth and a net-worth-over-time graph.
- **Refresh** — re-fetch balances for all linked Plaid accounts.
- **Link New Account** — opens your browser to Plaid's hosted Link flow to
  connect a new bank or investment account. Requires a local callback
  server on `localhost:8080`, so it must be run somewhere you can open a
  browser to complete the flow.
- **Add Custom Account** — manually enter a name, balance, and type for an
  account without a Plaid connection.
- Below these, each linked institution or custom account has its own menu
  entry listing the accounts under it.

## Notes

- Plaid access tokens and account metadata are stored locally in the
  SQLite database; nothing is sent anywhere except to Plaid's API.
- Just a heads up, this is a personal project under active development that I
  like to use for learning C++ so there will be some rough spots.
