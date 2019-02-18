#include <stdexcept>

#include "make_unique.hpp"
#include "statistics-reader.h"

statistics_reader::statistics_reader(reader_real reader_mean,
                                     reader_real reader_dev,
                                     reader_count reader_count)
    : reader_mean_(std::move(reader_mean)),
      reader_dev_(std::move(reader_dev)),
      reader_count_(std::move(reader_count)),
      is_finished_()
{
}

void statistics_reader::next()
{
    reader_mean_.next(line_);
    reader_dev_.next(line_);
    reader_count_.next(line_);
    auto ok_mean = !reader_mean_.is_finished();
    auto ok_dev = !reader_dev_.is_finished();
    auto ok_count = !reader_count_.is_finished();

    if (ok_mean && ok_dev && ok_count) {
        if (reader_dev_.key() != reader_mean_.key() || reader_count_.key() != reader_mean_.key()) {
            throw std::runtime_error("input files have non-matching keys");
        }
        if (dev().size() != mean().size() || count().size() != mean().size()) {
            throw std::runtime_error("input files have different number of columns");
        }
        key_ = reader_mean_.key();
    } else if (!ok_mean && !ok_dev && !ok_count) {
        is_finished_ = true;
    } else {
        throw std::runtime_error("input files don't match in length");
    }
}

bool statistics_reader::is_finished() const { return is_finished_; }

const int64_t statistics_reader::key() const { return key_; }

const size_t statistics_reader::columns() const { return reader_mean_.columns(); }

const entry_real &statistics_reader::mean() const { return reader_mean_.current(); }

const entry_real &statistics_reader::dev() const { return reader_dev_.current(); }

const entry_count &statistics_reader::count() const { return reader_count_.current(); }
