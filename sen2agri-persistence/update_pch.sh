#!/bin/sh
rm -f pch.hpp
grep -Rh "^#include <" | LC_ALL=C sort -u > pch.hpp
