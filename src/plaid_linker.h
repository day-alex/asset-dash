#pragma once

#include "db.h"
#include "account_factory.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include "plaid_env.h"

using Json = nlohmann::json;

class PlaidLinker {
public:
  PlaidLinker() = delete;
  PlaidLinker(const std::string&, const std::string&, DB& db);
  void link_account();
  std::string get_public_token();
private:
  DB& db_;
  std::string client_id_;
  std::string secret_;
  std::string link_token_;
  httplib::Client cli_{"https://production.plaid.com"};

  std::string create_link_token();
  void wait_for_callback();
  void exchange_public_token(const std::string& public_token);
  void save_to_db(const std::string& access_token);
};
