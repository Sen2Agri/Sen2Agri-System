#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "entry.h"
#include "make_unique.hpp"

template <typename T>
class reader
{
    std::unique_ptr<std::ifstream> file_;
    std::int64_t key_;
    entry<T> entry_;
    bool is_finished_;

public:
    reader(std::unique_ptr<std::ifstream> file);

    void next(std::string &line);
    bool is_finished() const;
    int64_t key() const;
    size_t columns() const;
    const entry<T> &current() const;

private:
    bool read_entry(std::string &line);
};

using reader_real = reader<double>;
using reader_count = reader<uint64_t>;

template <typename T>
reader<T>::reader(std::unique_ptr<std::ifstream> file) : file_(std::move(file)), is_finished_()
{
}

template <typename T>
void reader<T>::next(std::string &line)
{
    if (!read_entry(line)) {
        is_finished_ = true;
    }
}

template <typename T>
bool reader<T>::is_finished() const
{
    return is_finished_;
}

template <typename T>
int64_t reader<T>::key() const
{
    return key_;
}

template <typename T>
size_t reader<T>::columns() const
{
    return entry_.size();
}

template <typename T>
const entry<T> &reader<T>::current() const
{
    return entry_;
}

template <typename T>
bool reader<T>::read_entry(std::string &line)
{
    if (!std::getline(*file_, line)) {
        return false;
    }
    std::stringstream ss(line);

    std::string value;
    if (!std::getline(ss, value, ',')) {
        throw std::runtime_error("incomplete file");
    }
    key_ = std::stoull(value);

    auto cols = std::count(line.begin(), line.end(), ',');
    entry_.resize(cols);
    size_t i = 0;
    while (std::getline(ss, value, ',')) {
        entry_[i++] = std::stod(value);
    }

    return true;
}
