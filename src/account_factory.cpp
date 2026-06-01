#include "account_factory.h"
#include "bank_account.h"
#include "investment_account.h"

std::unique_ptr<Account> make_account(const nlohmann::json& acct_info, const std::string& inst) {
  std::string acct_type { acct_info["type"] };
  if (acct_type == "investment") return std::make_unique<InvestmentAccount>(acct_info, inst);
  if (acct_type == "depository") return std::make_unique<BankAccount>(acct_info, inst);
  return std::make_unique<BankAccount>(acct_info, inst);
}
