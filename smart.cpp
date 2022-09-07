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
