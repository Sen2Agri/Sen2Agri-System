#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "entry.h"
#include "reader.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " count.csv count-1.csv [...]\n";
        return 1;
    }

    std::ofstream fcount(argv[1]);
    std::vector<reader_count> readers;

    readers.reserve(argc - 2);
    for (int i = 2; i < argc; i++) {
        std::cout << argv[i];
        readers.emplace_back(std::make_unique<std::ifstream>(argv[i]));
    }

    std::string line;
    for (auto &reader : readers) {
        reader.next(line);
    }

    while (true) {
        readers.erase(std::remove_if(readers.begin(), readers.end(),
                                     [](reader_count &r) { return r.is_finished(); }),
                      readers.end());
        if (readers.empty()) {
            break;
        }
        int64_t key = std::numeric_limits<int64_t>::max();
        auto columns = 0;
        for (const auto &reader : readers) {
            if (key > reader.key()) {
                key = reader.key();
                columns = reader.columns();
            }
        }

        entry_count count(columns);
        for (auto &reader : readers) {
            if (key == reader.key()) {
                count += reader.current();
                reader.next(line);
            }
        }

        fcount << key;
        for (size_t i = 0; i < columns; i++) {
            fcount << ',' << count[i];
        }
        fcount << '\n';
    }

    return 0;
}
