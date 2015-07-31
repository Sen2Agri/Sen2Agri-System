#pragma once

#include <pwd.h>

bool isUserInGroup(uid_t uid, const char *groupName);
