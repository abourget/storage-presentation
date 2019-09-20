



Blockchain state storage on Bitcoin, Ethereum
and EOSIO



Shanghai Meetup - September 19th 2019

        Alexandre Bourget
          CTO, dfuse.io




---------------------------------------

* Base comparison:
  * MySQL: tables, typed columns, primary keys, additional indexes
  * MongoDB: collections, schema-less (free-form fields), no joins,
    indexes per collections, manages around schemalessness
  * Both handle: INSERTS, UPDATES, DELETES are mutations on the
    tables/collections.
    * Similar to EOS, less similar to Bitcoin / Ethereum, as we'll see.

---

We will be looking at the transactions and how they change state, or
tables, or the data of the database.

We will be ignoring the blocks that transport them, how they converge
through consensus, the p2p propagation of those things.

---


# Bitcoin

> Code samples are from bitcoin-core's master branch from Sept 4th
> 2019.

* UTXO model, what is stored in the key-value store

* Graph for UTOX model

    Key:

    * src/txdb.cpp -> DB_COINS (prefixes) -> Serialize
    * src/primitives/transaction.h -> "class COutPoint"
    * `c[transaction_hash][index of vout]`

    Value:

    * src/coins.h -> "class Coin"
    * src/primitives/transaction.h -> "class CTxOut"
    * `[value in satoshis][script to run when spending][is coinbase?][block height where this was created]`

* A new transaction provides:

    * one of more: `[transaction_hash][index of vout]`
    * a `sigscript` that, when run, satifies the `[script to run when spending]`
    * a vector of UTXOs (`value in satoshis`, `script to run when spending`)
    * then the protocol creates new such pairs (new UTXOs), thereby
      transfering ownership.

OP_PUSH_32 "1123123f123f12f312f321f3" OP_CHECKSIG

* Important to remember here:

    * Simple key value
    * Fixed in its meaning
    * Underpins a cryptocurrency in its most basic form.

Bitcoin core has an internal wallet, and is able to keep track of the
balances for the coins it has the keys to unlock.

If you want to know the coins available for some random address, you
will need to query some other service, like dfuse.



# Ethereum

> Code samples are from go-ethereum's 1.9 release.

* Accounts hold (core/state/state_object.go)
  * Balance of Ether, the native currency
  * A nonce
  * Pointer to state storage root
  * Associated code

* Contract address scoped: bytes32 key -> bytes32 value
   * Smart Contract OPCODES
     core/vm/jump_table.go
   * `SSTORE`, `SLOAD`
     core/vm/instructions.go

* That is all that is known to the base layer storage engine.

* You might wonder, how do we implement maps, lists, etc. Here it is:

  `example.sol`

    * Position of variables, determine base index of storage:
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000001
      0000000000000000000000000000000000000000000000000000000000000002
      ...
    * With a mapping, you HASH the concatenation of the key, with its base storage location, ex:
      HASH(
        "0000000000000000000000000000000000000000000000000000000000000002"
      + "0000000000000000000000000e9a9755898dc4da9c9549decc0b8a19d2e072e2")
      = "a17f6670e83d131ac44d265cee28c4852720b5d350a3ec0c2f6cee1751af61b1" // wwuut?

    * One important conclusion of this "feature", is that you cannot iterate through key.  You
      always need to KNOW the key in order to fetch its value.

    * In any case, that big `a17f6670e83d131ac44d265cee28c485...`
      is meaningless externally.

    * You can keep a list of those keys in a separate array.

      "a17f6670e83d131ac44d265cee28c4852720b5d350a3ec0c2f6cee1751af61b1"
      "a17f6670e83d131ac44d265cee28c4852720b5d350a3ec0c2f6cee1751af61b2"
      "a17f6670e83d131ac44d265cee28c4852720b5d350a3ec0c2f6cee1751af61b3"
      "a17f6670e83d131ac44d265cee28c4852720b5d350a3ec0c2f6cee1751af61b4"

    * Logic lives in Solidity *compiler*, not the runtime... so some
      other creative contract compilers could use their own mapping
      algorithms, making it even more difficult to reverse-engineer,
      and make sense out of the data.

    * Some block explorers will show the state changes, but they're
      pretty opaque:

       https://etherscan.io/tx/0x082969e78b7c3ca3ea905d888710c131d2eece859673032a374dcc1b091941e7#statechange

     * The dfuse API provides that information too, and we are working
       to provide more meaning around the state.


* How does Ethereum allow you to search its state then? Like you would
  in a MySQL database, query some tables, and get some information?

    * Through read functions on the contract.  Forgot to expose some
      read functions? hrm.. well too bad.

    * However, Ethereum allows you to search what happens, is through Logs.

    * Similar to a print statement in a console program, except it
      respects a simple structure, with positional arguments.

    * You can only search those fields marked `indexed`, and that
      costs more gas.

    * You cannot change those fields afterwards, nor update your
      contract. So you hope you've made the right decision, and your
      needs don't change later on.  Or you can use `dfuse`, which
      indexes all the log fields.

    * To be efficient, the protocol uses Bloom filters, and adds those
      indexed fields into the bloom data structure.

    * A bloom filter is a data structure, 2048 bytes wide on Ethereum,
      that can say one of two things when searching for an indexed
      value:

        * I'm sure I've never seen that value
        * I possibly have seen that value

    * Allows it to be smaller in the blockchain.

    * You can therefore have false positives when searching the logs.

        * It can be very painful to re-check the results to make sure
          things match.

        * With dfuse, our searches are always hard matches.

    * Your code needs to make sure to sync logs and state changes, and
      keep those coherent.

    * Some people build state externally to the blockchain, based on
      logs, because it's the only thing that can be streamed out, and
      interpreted without calling the blockchain with explicit
      parameters to get some values.

This covers what you can write to the chain, how you can read it, and
what sort of data is available.


# EOSIO

> Code samples are from the v1.8 branch of the `eos` codebase.

The data layer on EOSIO chains is very interesting.

For each contract (deployed on an account), you can have millions of
tables, with a defined schema.

Let's look at the WebAssembly functions that are exposed by the
blockchain, which we call `intrinsics`.

    `libraries/chain/wasm_interface.cpp:1282` -> db_store_i64
    `libraries/chain/apply_context.cpp:644` -> db_store_i64

Each smart contract has its namespace of tables.

You can iterate through keys. (lowerbound, find, next, previous, etc..)

---

A contract can:
* only write to its own namespace.
* read tables from other contracts.

Tables are therefore namespaced as:

    [contract] / [scope] / [table]

* A `scope` can be any uint64 value, often used to have one table per
  user account, but not obligatory.

* A `table` is a readable name (still a uint64), but must be defined
  in the ABI to be useable... so you wouldn't use the account names
  for `table`.

Within each table, you can have 1.8e19 rows (18446744073709551616) that have:

    [uint64 primary key] -> [random number of bytes...]

In EOS, an account names (those 12 characters) maps to a `uint64`, so
it is very fitting.

Example: `eosio.token`:
* uses table name `stat` and `accounts`
    * `stat` uses the currency as scope (like `EOS`, name-encoded as
      `........ehbo5`), because there is only the need for one such
      table per currency
    * `accounts` uses one scope per account, and the primary key is the token it holds,
* such a token contract can therefore handle multiple currencies for each user account

These methods are available, but abstracted away by the C++
`multi_index` wrapper. Let's take a look at our example contract.

    main.hpp

We define:

* a contract, and its methods
* a structure, which will be binary-packed using an EOSIO-specific algorithm, called `FC_BUFFER`.
* a `multi_index` table, with a given _name_ (which will be referenced in the ABI)
* optional secondary indexes, like the `byaccount` index here
* a `primary_key` function to return a uint64
* other methods if we want secondary indexes

How we use it:

    main.cpp

* we get an instance of a table, with its scope (`member_table`)
* we can add rows with `emplace`
    * we specify who pays for the storage
    * we do the modifications
* we can fetch it through its secondary index, line 20
* modify it, and optionally change the payer, line 22
* remove the row, line 32

Annotations produce an ABI, which looks like:

    https://eosq.app/account/eosio.token/abi

and is stored _on-chain_, and is constantly used to interprete the
data of the rows in the database.

See how data is available from `eosq.app`:

    https://eosq.app/tx/8967168e3164748926b1c580839d2cb20996651b2526f09718012d5564897379

It makes reading the blockchain data a lot more feasible, and transparent.

---

Understanding the data layer, is, for the same reasons why you pick
MySQL over MongoDB or vice versa, often the main distinguisher in a
choice of technology.

---

Summary:

* Bitcoin: simple UTXO, creating a native currency model

* Ethereum: a key value store, limited to 32 bytes keys _and_ values,
  with compile-time decisions as to how storage will actually work,
  pretty opaque from outside

* EOSIO: full-blown tables, with iterable key values, very fast as all
  these tables are actually living in RAM of the nodes, with secondary
  indexes resolved both on-chain and externally. ABIs which help read
  and comprehend the data externally.

---

This should provide a good understanding of what is made possible by
the different platforms.

Hope you enjoyed!

Take a look at what we do at https://dfuse.io
