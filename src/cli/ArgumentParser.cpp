#include "../../include/xary/cli/ArgumentParser.hpp"
#include <iostream>
#include <vector>

namespace xary::cli {

Options ArgumentParser::parse(int argc, char *argv[]) {
  Options options;
  if (argc <= 1) {
    options.mode = Mode::Help;
    return options;
  }

  std::vector<std::string> args(argv + 1, argv + argc);

  for (std::size_t i = 0; i < args.size(); ++i) {
    const auto &arg = args[i];

    if (arg == "-h" || arg == "--help") {
      options.mode = Mode::Help;
      return options;
    } else if (arg == "-v" || arg == "--version") {
      options.mode = Mode::Version;
      return options;
    } else if (arg == "-e" || arg == "--encode") {
      if (i + 1 < args.size()) {
        options.mode = Mode::Encode;
        options.inputFile = args[++i];
      } else {
        options.isValid = false;
        options.errorMessage = "Missing input file path for --encode flag.";
        return options;
      }
    } else if (arg == "-d" || arg == "--decode") {
      if (i + 1 < args.size()) {
        options.mode = Mode::Decode;
        options.inputFile = args[++i];
      } else {
        options.isValid = false;
        options.errorMessage = "Missing input file path for --decode flag.";
        return options;
      }
    } else if (arg == "-o" || arg == "--output") {
      if (i + 1 < args.size()) {
        options.outputFile = args[++i];
      } else {
        options.isValid = false;
        options.errorMessage = "Missing output file path for --output flag.";
        return options;
      }
    } else if (arg == "-i" || arg == "--inspect") {
      if (i + 1 < args.size()) {
        options.mode = Mode::Inspect;
        options.inputFile = args[++i];
      } else {
        options.isValid = false;
        options.errorMessage = "Missing target file path for --inspect flag.";
        return options;
      }
    } else {
      options.isValid = false;
      options.errorMessage = "Unknown argument or flag: " + arg;
      return options;
    }
  }

  return options;
}

void ArgumentParser::printHelp() {
  std::cout
      << "Usage: xary [OPTIONS]\n\n"
      << "Options:\n"
      << "  -h, --help                 Display this help menu and exit\n"
      << "  -v, --version              Display engine version\n"
      << "  -e, --encode <file>        Encode binary file in 64 KB chunks\n"
      << "  -d, --decode <file>        Decode binary file in 64 KB chunks\n"
			<< "  -i, --inspect <file>       Inspect file magic signature & detect real extension\n"
      << "  -o, --output <file>        Specify custom output file path\n\n"
      << "Examples:\n"
      << "  xary --version\n"
			<< "  xary -i corrupted_file.png\n"
      << "  xary -e data.bin\n"
      << "  xary -e data.bin -o encoded.xary\n"
      << "  xary -d encoded.xary -o restored.bin\n";
}

void ArgumentParser::printVersion() {
  std::cout << "Xary Binary Engine v0.1.0 (C++20)\n";
}

} // namespace xary::cli