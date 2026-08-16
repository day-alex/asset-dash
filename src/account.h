#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <optional>

using json = nlohmann::json;

class Account {
public:
    Account(const json& acct_info, const std::string& inst);
    virtual ~Account() = default;

protected:
    Account(std::string account_id, std::string name, std::string institution, double balance);

public:

    const std::string& account_id() const { return account_id_; }
    const std::string& name() const { return name_; }
    const std::string& institution() const { return institution_; }
    const std::optional<std::string>& subtype() const { return subtype_; }
    double balance() const { return balance_; }

    const std::string& access_token() const { return access_token_; }
    void set_access_token(const std::string& access_token) { access_token_ = access_token; }

    virtual std::string type_label() const = 0;
    virtual std::string summary_str() const;

protected:
    std::string account_id_;
    std::string name_;
    std::string institution_;
    std::optional<std::string> subtype_;
    double balance_;
    std::string access_token_;
};
