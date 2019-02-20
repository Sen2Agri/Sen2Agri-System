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
entry<T> splice(int dates, const entry<T> &e1, const entry<T> &e2)
{
    entry<T> r(dates * 11);
    for (int i = 0; i < dates; i++) {
        r[i * 11] = e1[i];
        r[i * 11 + 1] = e1[i * 7 + 1];
        r[i * 11 + 2] = e1[i * 7 + 2];
        r[i * 11 + 3] = e1[i * 7 + 3];
        r[i * 11 + 4] = e2[i * 4];
        r[i * 11 + 5] = e2[i * 4 + 1];
        r[i * 11 + 6] = e2[i * 4 + 2];
        r[i * 11 + 7] = e2[i * 4 + 3];
        r[i * 11 + 8] = e1[i * 7 + 4];
        r[i * 11 + 9] = e1[i * 7 + 5];
        r[i * 11 + 10] = e1[i * 7 + 6];
    }
    return r;
}

int main(int argc, char *argv[])
{
    // if (argc % 3 != 1 || argc < 7) {
    //     std::cerr << "Usage: " << argv[0]
    //               << " mean.csv dev.csv count.csv mean.csv dev.csv count.csv mean-re.csv
    //               dev-re.csv count-re.csv\n";
    //     return 1;
    // }

    // std::ofstream fmean(argv[1]);
    // std::ofstream fdev(argv[2]);
    // std::ofstream fcount(argv[3]);

    // statistics_reader r1{std::ifstream(argv[4]), std::ifstream(argv[5]), std::ifstream(argv[6])};
    // statistics_reader r2{std::ifstream(argv[7]), std::ifstream(argv[8]), std::ifstream(argv[9])};

    // entry_real e;
    // while (true) {
    //     r1.next();
    //     r2.next();

    //     if (r1.is_finished() && r2.is_finished()) {
    //         break;
    //     }
    //     if (r1.is_finished() != r2.is_finished()) {
    //         throw std::runtime_error("incomplete file");
    //     }

    //     if (r1.key() != r2.key()) {
    //         throw std::runtime_error("key mismatch");
    //     }

    //     auto dates = r1.columns() / 7;
    //     auto key = r1.key();
    //     auto r_mean = splice(dates, r1.mean(), r2.mean());
    //     auto r_dev = splice(dates, r1.dev(), r2.dev());
    //     auto r_count = splice(dates, r1.count(), r2.count());

    //     // std::cout << dates << ' ' << r1.mean().size() << ' ' << r1.dev().size() << ' ' <<
    //     r1.count().size() << '\n';
    //     // std::cout << dates << ' ' << r2.mean().size() << ' ' << r2.dev().size() << ' ' <<
    //     r2.count().size() << '\n';
    //     // std::cout << dates << ' ' << r_mean.size() << ' ' << r_dev.size() << ' ' <<
    //     r_count.size() << '\n'; fmean << key; for (size_t i = 0; i < r_mean.size(); i++) {
    //         fmean << ',' << r_mean[i];
    //     }
    //     fmean << '\n';

    //     fdev << key;
    //     for (size_t i = 0; i < r_dev.size(); i++) {
    //         fdev << ',' << r_dev[i];
    //     }
    //     fdev << '\n';

    //     fcount << key;
    //     for (size_t i = 0; i < r_count.size(); i++) {
    //         fcount << ',' << r_count[i];
    //     }
    //     fcount << '\n';
    // }

    if (argc % 3 != 0 || argc < 6) {
        std::cerr << "Usage: " << argv[0]
                  << " mean.csv dev.csv mean-1.csv dev-1.csv count-1.csv [...]\n";
        return 1;
    }

    std::vector<statistics_reader> statistics_readers;
    statistics_readers.reserve(argc / 3 - 1);

    std::ofstream fmean(argv[1]);
    std::ofstream fdev(argv[2]);

    for (int i = 3; i < argc; i += 3) {
        auto mean = argv[i];
        auto dev = argv[i + 1];
        auto count = argv[i + 2];
        statistics_readers.emplace_back(std::make_unique<std::ifstream>(mean),
                                        std::make_unique<std::ifstream>(dev),
                                        std::make_unique<std::ifstream>(count));
    }

    for (auto &statistics_reader : statistics_readers) {
        statistics_reader.next();
    }

    // https://stats.stackexchange.com/a/56000
    while (true) {
        statistics_readers.erase(
            std::remove_if(statistics_readers.begin(), statistics_readers.end(),
                           [](statistics_reader &r) { return r.is_finished(); }),
            statistics_readers.end());
        if (statistics_readers.empty()) {
            break;
        }

        auto key = std::numeric_limits<int64_t>::max();
        auto columns = 0;
        for (const auto &statistics_reader : statistics_readers) {
            if (key > statistics_reader.key()) {
                key = statistics_reader.key();
                columns = statistics_reader.columns();
            }
        }

        entry_real sum(columns);
        entry_count count(columns);
        for (auto &statistics_reader : statistics_readers) {
            if (key == statistics_reader.key()) {
                sum += statistics_reader.mean() * statistics_reader.count();
                count += statistics_reader.count();
            }
        }

        entry_real dev(columns);
        for (auto &statistics_reader : statistics_readers) {
            if (key == statistics_reader.key()) {
                for (size_t i = 0; i < columns; i++) {
                    auto m = statistics_reader.mean()[i];
                    auto d = statistics_reader.dev()[i];
                    auto c = statistics_reader.count()[i];
                    if (count[i] > 0) {
                        if (c > 1) {
                            auto mc = sum[i] / count[i];
                            dev[i] += (c - 1) * d * d + c * (m - mc) * (m - mc);
                        } else {
                            auto mc = sum[i] / count[i];
                            dev[i] += c * d * d + c * (m - mc) * (m - mc);
                        }
                    } else {
                        dev[i] = 0;
                    }
                }
                statistics_reader.next();
            }
        }

        for (size_t i = 0; i < columns; i++) {
            if (count[i] > 1) {
                dev[i] /= count[i] - 1;
            }
            dev[i] = std::sqrt(dev[i]);
        }

        fmean << key;
        for (size_t i = 0; i < columns; i++) {
            if (count[i] > 0) {
                fmean << ',' << sum[i] / count[i];
            } else {
                fmean << ',' << 0;
            }
        }
        fmean << '\n';

        fdev << key;
        for (size_t i = 0; i < columns; i++) {
            fdev << ',' << dev[i];
        }
        fdev << '\n';
    }
    return 0;
}
