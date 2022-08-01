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
    void dfs(name code,asset quantity,std::string memo){
      require_auth(operate_account);
      transfer(code,operate_account,name("onesgamedefi"),quantity,memo);
    }
    [[eosio::action]]
    void defi(name code,asset quantity,std::string memo){
      require_auth(operate_account);
      transfer(code,operate_account,name("swap.defi"),quantity,memo);
    }
    [[eosio::action]]
    void dfssell(name code,asset before,std::string memo){
      //增量的代币兑换,before是兑换前的代币,需要用到last表,故而使用前需要savebalance
      require_auth(operate_account);
      asset result=get_last_balance(before.symbol.code());
      asset delta=result-before;
      action(
        permission_level{operate_account, "active"_n},
        name(get_self()), 
        "dfs"_n, 
        std::make_tuple(code,delta,memo)
      ).send(); 
    }
    [[eosio::action]]
    void defisell(name code,asset before,std::string memo){
      //增量的代币兑换,before是兑换前的代币,需要用到last表,故而使用前需要savebalance
      require_auth(operate_account);
      asset result=get_last_balance(before.symbol.code());
      asset delta=result-before;
      action(
        permission_level{operate_account, "active"_n},
        name(get_self()), 
        "defi"_n, 
        std::make_tuple(code,delta,memo)
      ).send(); 
    } 
   
    [[eosio::action]]
    void trade(uint64_t dfs_id,uint64_t defibox_id,uint64_t profit,int64_t min_amount,bool is_reverse){//is_reverse是因为有些交易对是相反的
      require_auth(call_account);
      const double_t fee=0.006;//两个交易所手续费
      auto ones_pair=get_dfs_pairs(bug_id);
      pair defibox_pair=get_defibox_pairs(defibox_id);
      double_t price;
      if(is_reverse==false) price=(double_t)defibox_pair.reserve1.amount/(double_t)defibox_pair.reserve0.amount;
      else price=(double_t)defibox_pair.reserve0.amount/(double_t)defibox_pair.reserve1.amount;

      int64_t amount=(double_t)(ones_pair.reserve0.amount)-sqrt((double_t)(ones_pair.reserve0.amount))*sqrt((double_t)(ones_pair.reserve1.amount))/sqrt((price*(1.0-fee)));
      asset swap_eos_quantity=ones_pair.reserve0;
      asset max_eos=get_balance(ones_pair.token0.get_contract(),operate_account,ones_pair.token0.get_symbol().code());
      
      if(amount>0){//EOS/USDT交易对在ONES更便宜
        swap_eos_quantity.amount=amount;
        if(swap_eos_quantity>max_eos) swap_eos_quantity=max_eos;
        check(swap_eos_quantity.amount>=min_amount||-swap_eos_quantity.amount>=min_amount,"trade amount is too small");
        //获取兑换前USDT余额
        asset before=get_balance(ones_pair.token1.get_contract(),operate_account,ones_pair.token1.get_symbol().code());
        //保存兑换前的EOS余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "savebalance"_n, 
          std::make_tuple(ones_pair.token0.get_contract(),ones_pair.token0.get_symbol().code())
        ).send();
        //把EOS换成USDT
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "exbox"_n, 
          std::make_tuple(ones_pair.token0.get_contract(),swap_eos_quantity,std::string("swap,0,")+std::to_string(defibox_id))
        ).send();  
        //保存兑换后的USDT余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "savebalance"_n, 
          std::make_tuple(ones_pair.token1.get_contract(),ones_pair.token1.get_symbol().code())
        ).send();
        //把USDT换成EOS  
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "exbugsell"_n, 
          std::make_tuple(ones_pair.token1.get_contract(),before,std::string("swap:")+std::to_string(bug_id)+std::string(",min:0"))//只能支持单路径交易
        ).send(); 
        //检查余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "checkbalance"_n, 
          std::make_tuple(ones_pair.token0.get_contract(),ones_pair.token0.get_symbol().code(),profit)
        ).send();
      }else{//EOS/USDT交易对在ONES更贵
        amount=(double_t)(ones_pair.reserve0.amount)-sqrt((double_t)(ones_pair.reserve0.amount))*sqrt((double_t)(ones_pair.reserve1.amount))/sqrt((price*(1.0+fee)));
        swap_eos_quantity.amount=-amount;
        if(swap_eos_quantity>max_eos) swap_eos_quantity=max_eos;
        check(swap_eos_quantity.amount>=min_amount||-swap_eos_quantity.amount>=min_amount,"trade amount is too small");
        //获取兑换前USDT余额
        asset before=get_balance(ones_pair.token1.get_contract(),operate_account,ones_pair.token1.get_symbol().code());
        //保存兑换前的EOS余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "savebalance"_n, 
          std::make_tuple(ones_pair.token0.get_contract(),ones_pair.token0.get_symbol().code())
        ).send();
        //把EOS换成USDT
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "exburger"_n, 
          std::make_tuple(ones_pair.token0.get_contract(),swap_eos_quantity,std::string("swap:")+std::to_string(bug_id)+std::string(",min:0"))
        ).send();  
        //保存兑换后的USDT余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "savebalance"_n, 
          std::make_tuple(ones_pair.token1.get_contract(),ones_pair.token1.get_symbol().code())
        ).send();
        //把USDT换成EOS  
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "exboxsell"_n, 
          std::make_tuple(ones_pair.token1.get_contract(),before,std::string("swap,0,")+std::to_string(defibox_id))
        ).send(); 
        //检查余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "checkbalance"_n, 
          std::make_tuple(ones_pair.token0.get_contract(),ones_pair.token0.get_symbol().code(),profit)
        ).send();
      }
    }    