#!/bin/sh
function format_folder() {
    folder=$1
    echo "Formatting files in ${folder}"
    find $folder -name "*.h" -exec clang-format -i {} +
    find $folder -name "*.hpp" -exec clang-format -i {} +
    find $folder -name "*.cpp" -exec clang-format -i {} +
}

format_folder "sen2agri-common"
format_folder "sen2agri-config"
format_folder "sen2agri-http-listener"
format_folder "sen2agri-orchestrator"
format_folder "sen2agri-persistence"
