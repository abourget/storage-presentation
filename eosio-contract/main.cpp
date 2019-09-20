#include "main.hpp"

void battlefield::insertdata(name account) {
    require_auth(account);

    members member_table(get_self(), "scope1"_n);

    member_table.emplace(account /* payer */, [&](auto& row) {
        row.id = 1;
        row.account = "account2"_n;
        row.memo = "insert billed to calling account";
        row.created_at = time_point_sec(current_time_point());
    });
}

void battlefield::updatedata(name account) {
    //require_auth(account);

    members member_table(get_self(), "scope1"_n);

    auto index = member_table.template get_index<"byaccount"_n>();

    auto itr1 = index.find("account2"_n.value);

    index.modify(itr1, get_self() /* payer */, [&](auto& row) {
        row.memo = "updated row 1";
    });
}

void battlefield::removedata(name account) {
    //require_auth(account);

    members member_table(get_self(), "scope1"_n);

    auto index = member_table.template get_index<"byaccount"_n>();

    index.erase(index.find("account2"_n.value));
}
