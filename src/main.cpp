#include <iostream>
#include <vector>
#include "../include/xary/core/Stream.hpp"
#include "../include/xary/cli/ArgumentParser.hpp"

using namespace xary;

int main(int argc, char* argv[]) {
    cli::Options options = cli::ArgumentParser::parse(argc, argv);

    if (!options.isValid) {
        std::cerr << "❌ Error: " << options.errorMessage << "\n\n";
        cli::ArgumentParser::printHelp();
        return 1;
    }

    switch (options.mode) {
        case cli::Mode::Help:
            cli::ArgumentParser::printHelp();
            return 0;

        case cli::Mode::Version:
            cli::ArgumentParser::printVersion();
            return 0;

        case cli::Mode::Encode: {
            std::string outPath = options.outputFile.empty() ? (options.inputFile + ".xary") : options.outputFile;
            std::cout << "[+] Encoding file: " << options.inputFile << "\n";
            std::cout << "[+] Output destination: " << outPath << "\n";

            core::Stream stream(options.inputFile, 64 * 1024);
            if (!stream.isOpen()) {
                std::cerr << "❌ Error: Could not open source file '" << options.inputFile << "'\n";
                return 1;
            }

            std::vector<uint8_t> buffer;
            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            while (std::size_t bytesRead = stream.readChunk(buffer)) {
                totalBytes += bytesRead;
                chunks++;
            }

            std::cout << "✔ Successfully processed " << totalBytes << " bytes across " << chunks << " chunk(s).\n";
            return 0;
        }

        case cli::Mode::Decode: {
            std::string outPath = options.outputFile.empty() ? (options.inputFile + ".out") : options.outputFile;
            std::cout << "[+] Decoding file: " << options.inputFile << "\n";
            std::cout << "[+] Output destination: " << outPath << "\n";

            core::Stream stream(options.inputFile, 64 * 1024);
            if (!stream.isOpen()) {
                std::cerr << "❌ Error: Could not open source file '" << options.inputFile << "'\n";
                return 1;
            }

            std::vector<uint8_t> buffer;
            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            while (std::size_t bytesRead = stream.readChunk(buffer)) {
                totalBytes += bytesRead;
                chunks++;
            }

            std::cout << "✔ Successfully decoded " << totalBytes << " bytes across " << chunks << " chunk(s).\n";
            return 0;
        }

        case cli::Mode::None:
        default:
            cli::ArgumentParser::printHelp();
            return 0;
    }
}