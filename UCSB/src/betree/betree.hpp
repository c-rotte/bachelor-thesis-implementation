#pragma once

#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <string>

#include "src/core/db.hpp"
#include "src/core/helper.hpp"
#include "src/core/types.hpp"

#include "src/betree/BeTree.h"

#define BLOCK_SIZE 4096
#define PAGE_AMOUNT 122070
#define EPSILON 50
#define GROWTH_FACTOR 1.5
#define VALUE_SIZE_BYTES 100

namespace betree {

namespace fs = ucsb::fs;

using key_t = ucsb::key_t;
using keys_spanc_t = ucsb::keys_spanc_t;
using value_span_t = ucsb::value_span_t;
using value_spanc_t = ucsb::value_spanc_t;
using values_span_t = ucsb::values_span_t;
using values_spanc_t = ucsb::values_spanc_t;
using value_lengths_spanc_t = ucsb::value_lengths_spanc_t;
using operation_status_t = ucsb::operation_status_t;
using operation_result_t = ucsb::operation_result_t;
using transaction_t = ucsb::transaction_t;

inline std::array<unsigned char, VALUE_SIZE_BYTES> toArray(value_span_t& valueSpan) {
    assert(valueSpan.size() == VALUE_SIZE_BYTES);
    std::array<unsigned char, VALUE_SIZE_BYTES> valueArray;
    memcpy(valueArray.data(), valueSpan.data(), VALUE_SIZE_BYTES);
    return valueArray;
}

inline std::array<unsigned char, VALUE_SIZE_BYTES> toArray(value_spanc_t& valueSpan) {
    assert(valueSpan.size() == VALUE_SIZE_BYTES);
    std::array<unsigned char, VALUE_SIZE_BYTES> valueArray;
    memcpy(valueArray.data(), valueSpan.data(), VALUE_SIZE_BYTES);
    return valueArray;
}

struct betree_t : public ucsb::db_t {
public:
    inline betree_t() : db_(nullptr) {}
    inline ~betree_t() { close(); }

    void set_config(fs::path const& config_path, fs::path const& dir_path) override;
    bool open() override;
    bool close() override;
    void destroy() override;

    operation_result_t upsert(key_t key, value_spanc_t value) override;
    operation_result_t update(key_t key, value_spanc_t value) override;
    operation_result_t remove(key_t key) override;

    operation_result_t read(key_t key, value_span_t value) const override;
    operation_result_t batch_upsert(keys_spanc_t keys, values_spanc_t values, value_lengths_spanc_t sizes) override;
    operation_result_t batch_read(keys_spanc_t keys, values_span_t values) const override;

    operation_result_t bulk_load(keys_spanc_t keys, values_spanc_t values, value_lengths_spanc_t sizes) override;

    operation_result_t range_select(key_t key, size_t length, values_span_t values) const override;
    operation_result_t scan(key_t key, size_t length, value_span_t single_value) const override;

    void flush() override;
    size_t size_on_disk() const override;

    std::unique_ptr<transaction_t> create_transaction() override;

private:
    struct config_t {
        size_t write_buffer_size = 0;
        size_t max_file_size = 0;
        size_t max_open_files = -1;
        std::string compression;
        size_t cache_size = 0;
        size_t filter_bits = -1;
    };

    fs::path config_path_;
    fs::path dir_path_;

    std::unique_ptr<BeTree<key_t, std::array<unsigned char, VALUE_SIZE_BYTES>,
                           BLOCK_SIZE, PAGE_AMOUNT, EPSILON>>
            db_;
};

void betree_t::set_config(fs::path const& config_path, fs::path const& dir_path) {
    config_path_ = config_path;
    dir_path_ = dir_path;
}

bool betree_t::open() {
    if (db_)
        return true;

    db_ = std::make_unique<
            BeTree<key_t, std::array<unsigned char, VALUE_SIZE_BYTES>,
                   BLOCK_SIZE, PAGE_AMOUNT, EPSILON>>(
            dir_path_.string(), GROWTH_FACTOR);

    return true;
}

bool betree_t::close() {
    db_->flush();
    db_.reset(nullptr);
    return true;
}

void betree_t::destroy() {
    bool ok = close();
    assert(ok);
    fs::remove_all(dir_path_);
}

operation_result_t betree_t::upsert(key_t key, value_spanc_t value) {
    db_->insert(key, std::move(toArray(value)));
    return {1, operation_status_t::ok_k};
}

operation_result_t betree_t::update(key_t key, value_spanc_t value) {

    /*
    auto res = db_->find(key);
    if (!res)
        return {1, operation_status_t::not_found_k};
    */

    db_->update(key, std::move(toArray(value)));

    return {1, operation_status_t::ok_k};
}

operation_result_t betree_t::remove(key_t key) {
    db_->erase(key);
    return {1, operation_status_t::ok_k};
}

operation_result_t betree_t::read(key_t key, value_span_t value) const {

    auto res = db_->find(key);

    if (!res)
        return {1, operation_status_t::not_found_k};

    memmove(value.data(), res->data(), VALUE_SIZE_BYTES);

    return {1, operation_status_t::ok_k};
}

operation_result_t betree_t::batch_upsert(keys_spanc_t keys, values_spanc_t values, value_lengths_spanc_t sizes) {
    return {keys.size(), operation_status_t::error_k};
}

operation_result_t betree_t::batch_read(keys_spanc_t keys, values_span_t values) const {
    return {keys.size(), operation_status_t::error_k};
}

operation_result_t betree_t::bulk_load(keys_spanc_t keys, values_spanc_t values, value_lengths_spanc_t sizes) {

    size_t offset = 0;
    for (std::size_t i = 0; i < keys.size(); i++) {
        auto subspan = values.subspan(offset, sizes[i]);
        db_->insert(keys[0], std::move(toArray(subspan)));
        offset += sizes[i];
    }

    return {keys.size(), operation_status_t::ok_k};
}

operation_result_t betree_t::range_select(key_t key, size_t length, values_span_t values) const {
    return {0, operation_status_t::error_k};
}

operation_result_t betree_t::scan(key_t key, size_t length, value_span_t single_value) const {
    return {0, operation_status_t::error_k};
}

void betree_t::flush() {
    db_->flush();
}

size_t betree_t::size_on_disk() const {
    return ucsb::size_on_disk(dir_path_);
}

std::unique_ptr<transaction_t> betree_t::create_transaction() {
    return {};
}

}// namespace betree