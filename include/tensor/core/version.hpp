// SPDX-License-Identifier: MIT
#pragma once

#include <string_view>

#define TENSOR_VERSION_MAJOR 0
#define TENSOR_VERSION_MINOR 0
#define TENSOR_VERSION_PATCH 1
#define TENSOR_VERSION_PRERELEASE "alpha"

namespace tensor {

struct Version {
    int major;
    int minor;
    int patch;
    std::string_view prerelease;
};

inline constexpr Version version{
    TENSOR_VERSION_MAJOR,
    TENSOR_VERSION_MINOR,
    TENSOR_VERSION_PATCH,
    TENSOR_VERSION_PRERELEASE,
};

}  // namespace tensor
