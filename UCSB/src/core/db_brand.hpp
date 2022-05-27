#pragma once

#include "src/core/types.hpp"
#include "src/core/db.hpp"

#include "src/rocksdb/rocksdb.hpp"
#include "src/leveldb/leveldb.hpp"
#include "src/wiredtiger/wiredtiger.hpp"
#include "src/lmdb/lmdb.hpp"
#include "src/betree/betree.hpp"
#include "src/btree/btree.hpp"

namespace ucsb {

enum class db_brand_t {
    unknown_k,

    rocksdb_k,
    leveldb_k,
    wiredtiger_k,
    lmdb_k,
    betree_k,
    btree_k
};

inline std::shared_ptr<db_t> make_db(db_brand_t db_brand, bool transactional) {
    if (transactional) {
        switch (db_brand) {
        case db_brand_t::rocksdb_k: return std::make_shared<facebook::rocksdb_t>(facebook::db_mode_t::transactional_k);
        default: break;
        }
    }
    else {
        switch (db_brand) {
        case db_brand_t::rocksdb_k: return std::make_shared<facebook::rocksdb_t>(facebook::db_mode_t::regular_k);
        case db_brand_t::leveldb_k: return std::make_shared<google::leveldb_t>();
        case db_brand_t::wiredtiger_k: return std::make_shared<mongodb::wiredtiger_t>();
        case db_brand_t::lmdb_k: return std::make_shared<symas::lmdb_t>();
        case db_brand_t::betree_k: return std::make_shared<betree::betree_t>();
        case db_brand_t::btree_k: return std::make_shared<btree::btree_t>();
        default: break;
        }
    }
    return {};
}

inline db_brand_t parse_db_brand(std::string const& name) {
    if (name == "rocksdb")
        return db_brand_t::rocksdb_k;
    if (name == "leveldb")
        return db_brand_t::leveldb_k;
    if (name == "wiredtiger")
        return db_brand_t::wiredtiger_k;
    if (name == "lmdb")
        return db_brand_t::lmdb_k;
    if (name == "betree")
        return db_brand_t::betree_k;
    if (name == "btree")
        return db_brand_t::btree_k;
    return db_brand_t::unknown_k;
}

} // namespace ucsb