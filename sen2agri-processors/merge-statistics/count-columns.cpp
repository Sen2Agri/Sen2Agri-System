#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " file.csv [...]\n";
        return 1;
    }

    std::string line;
    for (int i = 1; i < argc; i++) {
        std::ifstream file(argv[i]);
        std::string line;

        std::cout << argv[i] << ' ';
        if (file >> line) {
            std::cout << std::count(line.begin(), line.end(), ',');
        } else {
            std::cout << "empty file";
        }
        std::cout << '\n';
    }

    return 0;
}
