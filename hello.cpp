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
      require_auth(call_account);

      dfs _dfsmarkets(name("defisswapcnt"), name("defisswapcnt").value);
      defi _defimarkets(name("swap.defi"), name("swap.defi").value);
      evo _evomarkets(name("evolutiondex"), name("evolutiondex").value);
      hbg _hbgmarkets(name("hamburgerswp"), name("hamburgerswp").value);
      pcash _pcashmarkets(name("swap.pcash"), name("swap.pcash").value);

      for(int i=0; i != _dfsmarkets.end(); i++){
      uint64_t bid = i;
      dfs base_pair=get_dfs_pairs(bid);
      symbol base_sym0 = base_pair.sym0;
      symbol base_sym1 = base_pair.sym1;
      name base_con0 = base_pair.contract0;
      name base_con1 = base_pair.contract1;
      asset base_res0 = base_pair.reserve0;
      asset base_res1 = base_pair.reserve1;
      std::string base_memo = std::string("swap:" + bid.to_string() + std::string(":0");
      for(int ii=0; ii != _defimarkets.end(); ii++){
      uint64_t qid = ii;
      defi base_pair=get_defi_pairs(qid);
      symbol quote_sym0 = base_pair.token0.symbol;
      symbol quote_sym1 = base_pair.token1.symbol;
      name quote_con0 = base_pair.token0.contract;
      name quote_con1 = base_pair.token1.contract;
      asset quote_res0 = base_pair.reserve0;
      asset quote_res1 = base_pair.reserve1;
      std::string quote_memo = std::string("swap,0," + qid.to_string();
      for(int ii=0; ii != _def
      if(quote_sym0==base_sym0 && quote_sym1==base_sym1 || quote_sym1==base_sym0 && quote_sym0==base_sym1 ) {
      trade(uint64_t bid,uint64_t qid,symbol base_sym0,symbol base_sym1,symbol quote_sym0,symbol quote_sym1,name base_con0,name base_con1,name quote_con0,name quote_con1,asset base_res0,asset base_res1,asset quote_res0,asset quote_res1,std::string base_memo,std::string quote_memo,uint64_t profit,int64_t min_amount);
    }
    }
   
   
    void trade(bid, qid, base_sym0, base_sym1, quote_sym0, quote_sym1, base_con0, base_con1, quote_con0, quote_con1, base_res0, base_res1, quote_res0, quote_res1, profit, min_amount){//is_reverse是因为有些交易对是相反

      require_auth(call_account);
      const double_t fee=0.006;//两个交易所手续费

      double_t price=(double_t)quote_res1.amount/(double_t)quote_res0.amount;

      int64_t amount=(double_t)(base_res0.amount)-sqrt((double_t)(base_res0.amount))*sqrt((double_t)(base_res1.amount))/sqrt((price*(1.0-fee)));
      asset swap_eos_quantity=base_res0;
      asset max_eos=get_balance(base_con0,operate_account,base_sym0.code());
      
      if(amount>0){//EOS/USDT交易对在ONES更便宜
        swap_eos_quantity.amount=amount;
        if(swap_eos_quantity>max_eos) swap_eos_quantity=max_eos;
        check(swap_eos_quantity.amount>=min_amount||-swap_eos_quantity.amount>=min_amount,"trade amount is too small");
        //获取兑换前USDT余额
        asset before=get_balance(base_con1,operate_account,base_sym1.code());
        //保存兑换前的EOS余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "savebalance"_n, 
          std::make_tuple(base_con0,base_sym0.code())
        ).send();
        //把EOS换成USDT
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "defi"_n, 
          std::make_tuple(base_con0,swap_eos_quantity,quote_memo)
        ).send();  
        //保存兑换后的USDT余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "savebalance"_n, 
          std::make_tuple(base_con1,base_sym1.code())
        ).send();
        //把USDT换成EOS  
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "dfssell"_n, 
          std::make_tuple(base_con1,before,base_memo)//只能支持单路径交易
        ).send(); 
        //检查余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "checkbalance"_n, 
          std::make_tuple(base_con0,base_sym0.code(),profit)
        ).send();
      }else{//EOS/USDT交易对在ONES更贵
        amount=(double_t)(ones_pair.reserve0.amount)-sqrt((double_t)(ones_pair.reserve0.amount))*sqrt((double_t)(ones_pair.reserve1.amount))/sqrt((price*(1.0+fee)));
        swap_eos_quantity.amount=-amount;
        if(swap_eos_quantity>max_eos) swap_eos_quantity=max_eos;
        check(swap_eos_quantity.amount>=min_amount||-swap_eos_quantity.amount>=min_amount,"trade amount is too small");
        //获取兑换前USDT余额
        asset before=get_balance(base_con1,operate_account,base_sym1.code());
        //保存兑换前的EOS余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "savebalance"_n, 
          std::make_tuple(base_con0,base_sym0.code())
        ).send();
        //把EOS换成USDT
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "dfs"_n, 
          std::make_tuple(base_con0,swap_eos_quantity,base_memo)
        ).send();  
        //保存兑换后的USDT余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "savebalance"_n, 
          std::make_tuple(base_con1,base_sym1.code())
        ).send();
        //把USDT换成EOS  
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "defisell"_n, 
          std::make_tuple(base_con1,before,quote_memo)
        ).send(); 
        //检查余额
        action(
          permission_level{operate_account, "active"_n},
          name(get_self()), 
          "checkbalance"_n, 
          std::make_tuple(base_con0,base_sym0.code(),profit)
        ).send();
      }
    } 


    private:

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

    //获取DFS表
    struct [[eosio::table]] dfs{
        uint64_t            mid;
        name                contract0;
        name                contract1;
        symbol              sym0;
        symbol              sym1;
        asset               reserve0;
        asset               reserve1;
        uint64_t            liquidity_token;
        double_t            price0_last;

        uint64_t primary_key() const { return mid; }
    };
    dfs get_dfs_pairs(uint64_t mid){
      eosio::multi_index< "markets"_n, market> liquidity_pairs_table(name("defisswapcnt"),name("defisswapcnt").value);
      auto it=liquidity_pairs_table.find(mid);
      check(it!=liquidity_pairs_table.end(),"N/A DFS pair id");
      return *it;
    }

    //获取DEFIBOX合约表
    struct token{
      name contract;
      symbol symbol;
    };
    struct [[eosio::table]] defi{
        uint64_t id;
        token token0;
        token token1;
        asset reserve0;
        asset reserve1;
        uint64_t liquidity_token;
        float_t price0_last;
        float_t price1_last;
        uint64_t price0_cumulative_last;
        uint64_t price1_cumulative_last;
        time_point_sec block_time_last;
        uint64_t primary_key()const { return id; }
    };
    defi get_defi_pairs(uint64_t id){
      eosio::multi_index< "pairs"_n, pair> liquidity_pairs_table(name("swap.defi"),name("swap.defi").value);
      auto it=liquidity_pairs_table.find(id);
      check(it!=liquidity_pairs_table.end(),"N/A DEFIBOX pair id");
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
