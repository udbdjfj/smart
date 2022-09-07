#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
using namespace eosio;

class [[eosio::contract]] goldmakerxxx : public contract {
  public:
    using contract::contract;

    static constexpr name operate_account=name("goldminerxxx");//change this field
    static constexpr name call_account=name("goldminerxxx");//change this field

    [[eosio::action]]
    void log(std::string message){
    }
    [[eosio::action]]
    void savebalance(name code,symbol_code sym_code){
      asset balance=get_balance(code,operate_account,sym_code);
      upsert(balance);
      //tmplog(balance.to_string());
    }
    [[eosio::action]]
    void checkbalance(name code,symbol_code sym_code,int64_t min_profit){
      asset after=get_balance(code,operate_account,sym_code);
      asset before=get_last_balance(sym_code);
      check(after.amount-before.amount>=min_profit,"profit not enough");
    }

    [[eosio::action]]
    void buy(name code,name buydex, asset quantity,std::string memo){
      require_auth(operate_account);
      transfer(code,operate_account,buydex,quantity,memo);
    }
    [[eosio::action]]
    void sell(name code, name selldex, asset before,std::string memo){
      //增量的代币兑换,before是兑换前的代币,需要用到last表,故而使用前需要savebalance
      require_auth(operate_account);
      asset result=get_last_balance(before.symbol.code());
      asset delta=result-before;
      action(
        permission_level{operate_account, "active"_n},
        name(get_self()), 
        "buy"_n, 
        std::make_tuple(code,delta,memo)
      ).send(); 
    }

    [[eosio::action]]
    void mine(uint64_t profit, int64_t min_amount){
      
    for (int i = 0; i < 5; i++) {
      eosio::name base_dex = get_dex(i);
        for (int i = 0; i < 5; i++) {
           eosio::name quote_dex = get_dex(i);

        }
    }
      
}

private:

    struct [[eosio::table]] dexes {
        uint64_t            id;
        name                dex_contract;

        uint64_t primary_key() const { return id; }
    };
    dexex get_dex(uint64_t id){
      eosio::multi_index< "dexes"_n, dexes> dex_dex(name("goldmakerxxx"),name("goldmakerxxx").value);
      auto it=dex_dex.find(id);
      check(it!=dex_dex.end(),"N/A DEX");
      return *it;
    }




