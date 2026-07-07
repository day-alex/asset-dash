#include "plaid_env.h"

PlaidEnv::PlaidEnv()
  : client_id_(load_env("PLAID_CLIENT_ID")),
    secret_(load_env("PLAID_PROD_SECRET")) {}

std::string PlaidEnv::load_env(const std::string& key) {
  std::ifstream file("../.env");
  std::string line;
  while (std::getline(file, line)) {
      auto pos = line.find('=');
      if (pos != std::string::npos && line.substr(0, pos) == key)
          return line.substr(pos + 1);
  }
  return "";
}
