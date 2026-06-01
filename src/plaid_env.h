#pragma once

#include <memory>
#include <fstream>
#include <string>
#include <vector>

class PlaidEnv {
public:
  PlaidEnv();
  const std::string client_id() const { return client_id_; }
  const std::string secret() const { return secret_; }
private:
  std::string client_id_;
  std::string secret_;
  std::string load_env(const std::string& key);
};
