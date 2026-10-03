#ifndef JUGGLER_HPP
#define JUGGLER_HPP
#pragma once

#include <string_view>

enum
{
    ERROR_OK            =  0,
    ERROR_FILE1_ACCESS  = -1,
    ERROR_FILE2_ACCESS  = -2,
    ERROR_FILE3_ACCESS  = -3,
    ERROR_WRONG_HASH    = -4,
    ERROR_SAME_HASH     = -5,
    ERROR_INVALID_FILE  = -6,
    ERROR_WRONG_OUTPUT_HASH = -7,
};

int create_patch(std::string_view file_original, std::string_view file_modified, std::string_view file_patch);
int apply_patch(std::string_view file_original, std::string_view file_patch, std::string_view file_modified);

#endif