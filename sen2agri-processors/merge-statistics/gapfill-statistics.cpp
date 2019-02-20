#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
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

    auto num_readers = statistics_readers.size();
    while (true) {
        auto all_done = true;
        auto some_done = false;
        for (auto &statistics_reader : statistics_readers) {
            statistics_reader.next();
            if (statistics_reader.is_finished()) {
                some_done = true;
            } else {
                all_done = false;
            }
        }

        if (all_done) {
            break;
        }
        if (some_done) {
            throw std::runtime_error("short input file");
        }

        auto key = statistics_readers[0].key();
        auto input_columns = statistics_readers[0].columns();
        for (const auto &statistics_reader : statistics_readers) {
            if (input_columns != statistics_reader.columns()) {
                throw std::runtime_error("different number of columns");
            }
            if (key != statistics_reader.key()) {
                throw std::runtime_error("non-matching keys");
            }
        }

        auto columns = num_readers * input_columns;

        entry_real mean(columns);
        entry_real dev(columns);
        entry_count count(columns);

        auto col = 0;
        for (auto &statistics_reader : statistics_readers) {
            copy_entry(mean, statistics_reader.mean(), col);
            copy_entry(dev, statistics_reader.dev(), col);
            copy_entry(count, statistics_reader.count(), col);

            col += statistics_reader.columns();
        }

        if (key == 0) {
            continue;
        }

        auto t = 0;
        for (size_t i = 0; i < columns; i++) {
            t += count[i];
        }

        if (t != 0) {
            for (ssize_t i = 0; i < num_readers; i++) {
                for (ssize_t j = 0; j < input_columns; j++) {
                    auto s = i * input_columns;
                    if (count[s + j] == 0) {
                        auto found_before = false;
                        size_t before;
                        for (ssize_t k = i - 1; k >= 0; k--) {
                            if (count[k * input_columns + j] != 0) {
                                found_before = true;
                                before = k;
                                break;
                            }
                        }

                        auto found_after = false;
                        size_t after;
                        for (ssize_t k = i + 1; k < num_readers; k++) {
                            if (count[k * input_columns + j] != 0) {
                                found_after = true;
                                after = k;
                                break;
                            }
                        }

                        if (found_before && !found_after) {
                            auto s_before = before * input_columns;
                            mean[s + j] = mean[s_before + j];
                            dev[s + j] = dev[s_before + j];
                            count[s + j] = count[s_before + j];
                        } else if (!found_before && found_after) {
                            auto s_after = after * input_columns;
                            mean[s + j] = mean[s_after + j];
                            dev[s + j] = dev[s_after + j];
                            count[s + j] = count[s_after + j];
                        } else if (found_before && found_after) {
                            auto s_before = before * input_columns;
                            auto s_after = after * input_columns;
                            auto dist_before = i - before;
                            auto dist_after = after - i;
                            auto dist = after - before;
                            mean[s + j] = (dist_after * mean[s_before + j] +
                                           dist_before * mean[s_after + j]) /
                                          dist;
                            dev[s + j] =
                                (dist_after * dev[s_before + j] + dist_before * dev[s_after + j]) /
                                dist;
                            count[s + j] = static_cast<size_t>(
                                (dist_after * static_cast<double>(count[s_before + j]) +
                                 dist_before * static_cast<double>(count[s_after + j])) /
                                    dist +
                                0.5);
                        }
                    }
                }
            }
        }

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
    return 0;
}
