#include "../../include/xary/cli/ArgumentParser.hpp"
#include <iostream>
#include <span>
#include <cstdlib>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Command Line Interface Parser (ArgumentParser.cpp)
 * Description : Implementation of zero-copy CLI parser utilizing std::string_view
 *               and std::span for heap-allocation-free token extraction.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::cli {

Options ArgumentParser::parse(int argc, const char* const argv[]) noexcept {
    Options options;
    if (argc <= 1) {
        options.mode = Mode::Help;
        return options;
    }

    // Zero-allocation slice over command-line token array
    const std::span<const char* const> args(argv + 1, static_cast<std::size_t>(argc - 1));

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg(args[i]);

        if (arg == "-h" || arg == "--help") {
            options.mode = Mode::Help;
            return options;
        }
        if (arg == "-v" || arg == "--version") {
            options.mode = Mode::Version;
            return options;
        }
        if (arg == "--sec" || arg == "--secure") {
            options.secureMode = true;
            continue;
        }

        // Inline parameter value reader for option flags requiring target paths
        auto captureValue = [&](std::string_view flagName, std::filesystem::path& targetPath) -> bool {
            if (i + 1 < args.size()) {
                targetPath = args[++i];
                return true;
            }
            options.isValid = false;
            options.errorMessage = "Missing parameter value for '" + std::string(flagName) + "' flag.";
            return false;
        };

        if (arg == "-e" || arg == "--encode") {
            options.mode = Mode::Encode;
            if (!captureValue(arg, options.inputFile)) return options;
        } else if (arg == "-d" || arg == "--decode") {
            options.mode = Mode::Decode;
            if (!captureValue(arg, options.inputFile)) return options;
        } else if (arg == "-i" || arg == "--inspect") {
            options.mode = Mode::Inspect;
            if (!captureValue(arg, options.inputFile)) return options;
        } else if (arg == "-p" || arg == "--pack") {
            options.mode = Mode::Pack;
            if (!captureValue(arg, options.inputFile)) return options;
        } else if (arg == "-u" || arg == "--unpack") {
            options.mode = Mode::Unpack;
            if (!captureValue(arg, options.inputFile)) return options;
        } else if (arg == "-o" || arg == "--output") {
            if (!captureValue(arg, options.outputFile)) return options;
        } else if (arg == "-k" || arg == "--key") {
            if (i + 1 < args.size()) {
                options.key = static_cast<uint32_t>(std::strtoul(args[++i], nullptr, 0));
            } else {
                options.isValid = false;
                options.errorMessage = "Missing 32-bit key parameter for '--key' flag.";
                return options;
            }
        } else {
            options.isValid = false;
            options.errorMessage = "Unrecognized command flag: '" + std::string(arg) + "'";
            return options;
        }
    }

    return options;
}

void ArgumentParser::printHelp() noexcept {
    std::cout
        << "======================================================================\n"
        << "                      Xary Binary Engine v1.0.0                       \n"
        << "======================================================================\n"
        << "Usage: xary [OPTIONS]\n\n"
        << "Core Engine Commands:\n"
        << "  -h, --help                 Display this help menu and exit\n"
        << "  -v, --version              Display engine build version\n"
        << "  -i, --inspect <file>       Inspect file magic signature & MIME info\n"
        << "  -e, --encode <file>        Encode target file in chunked binary stream\n"
        << "  -d, --decode <file>        Decode encrypted Xary binary stream\n\n"
        << "Archiving & Obfuscation Commands:\n"
        << "  -p, --pack <dir/file>      Pack folder or file into a Xary archive\n"
        << "  -u, --unpack <archive>     Extract Xary archive to output directory\n"
        << "  -o, --output <path>        Specify custom output destination path\n"
        << "  --sec, --secure            Enable stealth header obfuscation (--sec)\n"
        << "  -k, --key <hex/int>        Custom 32-bit key (Default: 0x5A9C3F11)\n\n"
        << "Examples:\n"
        << "  xary -i corrupted_file.png\n"
        << "  xary -e data.bin -o encoded.xary\n"
        << "  xary -p ./my_folder -o payload.xary --sec\n"
        << "  xary -u payload.xary -o ./extracted_folder\n\n";
}

void ArgumentParser::printVersion() noexcept {
    std::cout << "Xary Binary Engine v1.0.0 (C++20 Zero-Copy Pipeline)\n"
              << "Copyright (c) 2026 Piyush Rajput aka Harsh (DeveloperXHarsh). All rights reserved.\n";
}

} // namespace xary::cli
