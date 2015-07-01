#!/bin/sh
rm -f sen2agri-archiver/pch.hpp
grep -Rh "^#include <" sen2agri-archiver | LC_ALL=C sort -u > sen2agri-archiver/pch.hpp

rm -f sen2agri-common/pch.hpp
grep -Rh "^#include <" sen2agri-common | LC_ALL=C sort -u > sen2agri-common/pch.hpp

rm -f sen2agri-config/pch.hpp
grep -Rh "^#include <" sen2agri-config | LC_ALL=C sort -u > sen2agri-config/pch.hpp

rm -f sen2agri-executor/pch.hpp
grep -Rh "^#include <" sen2agri-executor | LC_ALL=C sort -u > sen2agri-executor/pch.hpp

rm -f sen2agri-http-listener/pch.hpp
grep -Rh "^#include <" sen2agri-http-listener | LC_ALL=C sort -u > sen2agri-http-listener/pch.hpp

rm -f sen2agri-orchestrator/pch.hpp
grep -Rh "^#include <" sen2agri-orchestrator | LC_ALL=C sort -u > sen2agri-orchestrator/pch.hpp

rm -f sen2agri-persistence/pch.hpp
grep -Rh "^#include <" sen2agri-persistence | LC_ALL=C sort -u > sen2agri-persistence/pch.hpp

rm -f sen2agri-processor-wrapper/pch.hpp
grep -Rh "^#include <" sen2agri-processor-wrapper | LC_ALL=C sort -u > sen2agri-processor-wrapper/pch.hpp
