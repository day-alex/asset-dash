#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <optional>

using json = nlohmann::json;

class Account {
public:
    Account(const json& acct_info, const std::string& inst);
    virtual ~Account() = default;

    const std::string& account_id() const { return account_id_; }
    const std::string& name() const { return name_; }
    const std::string& institution() const { return institution_; }
    const std::optional<std::string>& subtype() const { return subtype_; }
    double balance() const { return balance_; }

    virtual std::string type_label() const = 0;
    virtual void print_summary() const;

protected:
    std::string account_id_;
    std::string name_;
    std::string institution_;
    std::optional<std::string> subtype_;
    double balance_;
};
