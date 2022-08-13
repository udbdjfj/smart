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
    void mine(name base_dex){//is_reverse是因为有些交易对是相反

      eosio::multi_index< "dexes"_n, dexes> dexes_table(name("goldmakerxxx"),name("goldmakerxxx").value);

      auto rowit = dex_table.find(dexname);
      if(rowit==dex_table.end()) return {};
      name dexx;
      string dexxid;
      string dexxsym1;
      string dexxsym2;
      string dexxcon1;
      string dexxcon2;
      string dexxres1;
      string dexxres2;
      string get_base_pairs;
      for(auto& p: rowit->dexname){
       if(p.first.name==base_dex){
        base_dexx = p.first;
        base_dexxid = p.second;
        base_dexxsym1 = p.third;
        base_dexxsym2 = p.fourth;
        base_dexxcon1 = p.fifth;
        base_dexxcon2 = p.sixth;
        base_dexxres1 = p.seventh;
        base_dexxres2 = p.eighth;
        get_pairs["base"] = &std::string("get_") + p.first.to_string() + std::string("_pairs");

      require_auth(call_account);
      const double_t fee=0.006;//两个交易所手续费
      auto ones_pair=get_pairs["base"](base_dexxid);
      pair defibox_pair=get_defibox_pairs(defibox_id);
      double_t price;
      if(is_reverse==false) price=(double_t)defibox_pair.reserve1.amount/(double_t)defibox_pair.reserve0.amount;
      else price=(double_t)defibox_pair.reserve0.amount/(double_t)defibox_pair.reserve1.amount;

      int64_t amount=(double_t)(ones_pair.reserve0.amount)-sqrt((double_t)(ones_pair.reserve0.amount))*sqrt((double_t)(ones_pair.reserve1.amount))/sqrt((price*(1.0-fee)));
      asset swap_eos_quantity=ones_pair.reserve0;
      asset max_eos=get_balance(ones_pair.contract0,operate_account,ones_pair.sym0.code());
      
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
          "defi"_n, 
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
          "dfssell"_n, 
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
          "dfs"_n, 
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
          "defisell"_n, 
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
   }
  }
if(dexx=="") return {}; 
}


    void transfer(name code,name from,name to,asset quantity,std::string memo){
        action(
            permission_level{from, "active"_n},
            name(code), 
            "transfer"_n, 
            std::make_tuple(from,to,quantity,memo)
        ).send();     
    }
    void tmplog(std::string message){
      action(
        permission_level{get_self(), "active"_n},
        name(get_self()), 
        "log"_n, 
        std::make_tuple(message)
      ).send();  
    }
    struct [[eosio::table]] account {
        asset   balance;
        uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };

    static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
    {
        typedef eosio::multi_index< "accounts"_n, account > accounts;
        accounts accountstable( token_contract_account, owner.value );
        auto it = accountstable.find(sym_code.raw());
        check(it!=accountstable.end(),"Token not found, "+owner.to_string()+", "+sym_code.to_string()+"@"+token_contract_account.to_string());                        return it->balance;
    };

    typedef eosio::multi_index<"last"_n,account> balance;

    void upsert(asset now_balance){
        balance balancetable(get_self(),get_self().value);
        auto it=balancetable.find( now_balance.symbol.code().raw());
        if(it==balancetable.end()){
            balancetable.emplace(get_self(),[&](auto& row){
                row.balance=now_balance;
            });
        }else{
            balancetable.modify(it,get_self(),[&](auto& row){
                row.balance=now_balance;
            });
        }
    }

    asset get_last_balance(symbol_code sym_code){
        balance balancetable(get_self(),get_self().value);
        auto it=balancetable.find( sym_code.raw());
        return it->balance;
    }

    struct [[eosio::table]] dexes{
        name dexname;
        string dexid;
        string dexsym1;
        string dexsym2;
        string dexcon1;
        string dexcon2;
        string dexres1;
        string dexres2;

        name primary_key()const { return dexname; }
    };
    dexes get_dexes(name dexname){
      eosio::multi_index< "dexes"_n, dexes> dexes_table(name("goldmakerxxx"),name("goldmakerxxx").value);
      auto it=dexes_table.find(name);
      check(it!=dexes_table.end(),"N/A DEXES");
      return *it;
    }

    double_t sqrt(double_t A)  
    {   /**二分法实现开方
      需要注意的是：
      1.初始上界是A+0.25，而不是A
      2.double型的精度DBL_EPSILON，不能随意指定
      */
      const double_t DBL_EPSILON =2.2204460492503e-16;
      double_t a = 0.0, b = A + 0.25, m;  // b = A 是错误的上届
      // while(b - a > 2*DBL_EPSILON){  //sometimes dead cycle when m == a or m == b.
      for (;;)
      {
        m = (b + a) / 2;
        if (m - a < DBL_EPSILON || b - m < DBL_EPSILON) break;
        if ((m*m - A) * (a*a - A) < 0) b = m;
        else a = m;
      }
      return m;
    }
};
