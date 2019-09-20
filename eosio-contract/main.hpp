#pragma once

#include <algorithm>
#include <string>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>

using eosio::action;
using eosio::action_wrapper;
using eosio::asset;
using eosio::cancel_deferred;
using eosio::check;
using eosio::contract;
using eosio::const_mem_fun;
using eosio::current_time_point;
using eosio::datastream;
using eosio::indexed_by;
using eosio::name;
using eosio::permission_level;
using eosio::print;
using eosio::time_point_sec;
using std::function;
using std::string;

class [[eosio::contract("battlefield")]] battlefield : public contract {
    public:
        battlefield(name receiver, name code, datastream<const char*> ds)
        :contract(receiver, code, ds)
        {}

        [[eosio::action]]
        void insertdata(name account);

        [[eosio::action]]
        void updatedata(name account);

        [[eosio::action]]
        void removedata(name account);

    private:

        struct [[eosio::table]] member_row {
            uint64_t id;
            name account;
            asset amount;
            string memo;
            time_point_sec created_at;
            time_point_sec expires_at;

            auto primary_key() const { return id; }
            uint64_t by_account() const { return account.value; }
        };

        typedef eosio::multi_index<
            "member"_n, member_row,
            indexed_by<"byaccount"_n, const_mem_fun<member_row, uint64_t, &member_row::by_account>>
        > members;
};
