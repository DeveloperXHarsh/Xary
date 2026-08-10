#pragma once

#include <filesystem>
#include <string>
#include <cstdint>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Command Line Interface Parser (ArgumentParser.hpp)
 * Description : Zero-allocation CLI argument parser featuring compile-time view
 *               traversal, strict path validation, and type-safe execution modes.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::cli {

enum class Mode {
    None,
    Help,
    Version,
    Encode,
    Decode,
    Inspect,
    Pack,
    Unpack
};

struct Options {
    Mode mode{Mode::None};
    std::filesystem::path inputFile;
    std::filesystem::path outputFile;
    bool secureMode{false};
    uint32_t key{0x5A9C3F11};
    bool isValid{true};
    std::string errorMessage;
};

class ArgumentParser {
public:
    [[nodiscard]] static Options parse(int argc, const char* const argv[]) noexcept;
    static void printHelp() noexcept;
    static void printVersion() noexcept;
};

} // namespace xary::cli