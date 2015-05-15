#!/bin/sh
rm -f ../sen2agri-persistence/pch.hpp
grep -Rh "^#include <" ../sen2agri-persistence | LC_ALL=C sort -u > ../sen2agri-persistence/pch.hpp

rm -f ../sen2agri-config/pch.hpp
grep -Rh "^#include <" ../sen2agri-config | LC_ALL=C sort -u > ../sen2agri-config/pch.hpp
