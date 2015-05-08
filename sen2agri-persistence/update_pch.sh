#!/bin/sh
rm -f pch.hpp
grep -Rh "^#include <" | sort -u > pch.hpp
