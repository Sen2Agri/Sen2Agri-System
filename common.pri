CONFIG += c++11 precompile_header
PRECOMPILED_HEADER = pch.hpp

QMAKE_CXXFLAGS_DEBUG += -g3 -fsanitize=address -fno-omit-frame-pointer -O1
QMAKE_LFLAGS_DEBUG += -fsanitize=address
