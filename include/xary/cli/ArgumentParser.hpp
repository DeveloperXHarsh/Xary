#pragma once

#include <string>

namespace xary::cli {

enum class Mode {
    None,
    Help,
    Version,
    Encode,
    Decode
};

struct Options {
    Mode mode{Mode::None};
    std::string inputFile;
    std::string outputFile;
    bool isValid{true};
    std::string errorMessage;
};

class ArgumentParser {
public:
    static Options parse(int argc, char* argv[]);
    static void printHelp();
    static void printVersion();
};

}