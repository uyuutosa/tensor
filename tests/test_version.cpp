// SPDX-License-Identifier: MIT
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <tensor/tensor.hpp>

TEST_CASE("tensor::version reports the expected semver") {
    CHECK(tensor::version.major == 0);
    CHECK(tensor::version.minor == 0);
    CHECK(tensor::version.patch == 1);
    CHECK(tensor::version.prerelease == "alpha");
}
