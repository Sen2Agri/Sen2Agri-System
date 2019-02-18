#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "entry.h"
#include "reader.h"
#include "statistics-reader.h"

template <typename T>
void copy_entry(entry<T> &dst, const entry<T> &src, size_t position)
{
    for (size_t i = 0; i < src.size(); i++) {
        dst[position + i] = src[i];
    }
}

int main(int argc, char *argv[])
{
    if (argc % 3 != 1 || argc < 7) {
        std::cerr << "Usage: " << argv[0]
                  << " mean.csv dev.csv count.csv mean-1.csv dev-1.csv count-1.csv [...]\n";
        return 1;
    }

    std::vector<statistics_reader> statistics_readers;
    statistics_readers.reserve(argc / 3 - 1);

    std::ofstream fmean(argv[1]);
    std::ofstream fdev(argv[2]);
    std::ofstream fcount(argv[3]);

    for (int i = 4; i < argc; i += 3) {
        auto mean = argv[i];
        auto dev = argv[i + 1];
        auto count = argv[i + 2];
        statistics_readers.emplace_back(std::make_unique<std::ifstream>(mean),
                                        std::make_unique<std::ifstream>(dev),
                                        std::make_unique<std::ifstream>(count));
    }

    auto skipped = 0;
    while (true) {
        auto columns = 0;
        auto done = false;
        for (auto &statistics_reader : statistics_readers) {
            statistics_reader.next();
            if (statistics_reader.is_finished()) {
                done = true;
                break;
            }
            columns += statistics_reader.columns();
        }

        if (done) {
            break;
        }

        entry_real mean(columns);
        entry_real dev(columns);
        entry_count count(columns);

        auto skip = false;
        auto col = 0;
        auto key = statistics_readers[0].key();
        for (auto &statistics_reader : statistics_readers) {
            if (key != statistics_reader.key()) {
                throw std::runtime_error("non-matching keys");
            }

            auto cols = statistics_reader.columns();
            for (size_t i = 0; i < cols; i++) {
                if (statistics_reader.count()[i] == 0) {
                    skip = true;
                    break;
                }
            }

            if (skip) {
                break;
            }

            copy_entry(mean, statistics_reader.mean(), col);
            copy_entry(dev, statistics_reader.dev(), col);
            copy_entry(count, statistics_reader.count(), col);

            col += statistics_reader.columns();
        }

        if (skip) {
            skipped++;
            continue;
        }

        if (key != 0) {
            fmean << key;
            for (size_t i = 0; i < columns; i++) {
                fmean << ',' << mean[i];
            }
            fmean << '\n';

            fdev << key;
            for (size_t i = 0; i < columns; i++) {
                fdev << ',' << dev[i];
            }
            fdev << '\n';

            fcount << key;
            for (size_t i = 0; i < columns; i++) {
                fcount << ',' << count[i];
            }
            fcount << '\n';
        }
    }

    std::cout << skipped << " entries skipped\n";

    return 0;
}
