CONFIG += c++11 precompile_header
PRECOMPILED_HEADER = pch.hpp

QMAKE_CXXFLAGS_DEBUG += -g3 -fno-omit-frame-pointer -Wextra -fsanitize=address
QMAKE_LFLAGS_DEBUG += -fsanitize=address
