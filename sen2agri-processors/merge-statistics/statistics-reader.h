#pragma once

#include <cstdint>
#include <string>

#include "entry.h"
#include "reader.h"

class statistics_reader
{
    reader_real reader_mean_;
    reader_real reader_dev_;
    reader_count reader_count_;
    std::string line_;
    std::int64_t key_;
    bool is_finished_;

public:
    statistics_reader(reader_real reader_mean, reader_real reader_dev, reader_count reader_count);

    void next();
    bool is_finished() const;
    const int64_t key() const;
    const size_t columns() const;
    const entry_real &mean() const;
    const entry_real &dev() const;
    const entry_count &count() const;
};
