#include <iostream>
#include <vector>
#include "../include/xary/core/Stream.hpp"
#include "../include/xary/core/StreamWriter.hpp"
#include "../include/xary/cli/ArgumentParser.hpp"

using namespace xary;

// Lightweight 64-bit XOR mask key for chunk transformation
constexpr uint8_t XARY_XOR_KEY = 0x5A;

void transformBuffer(std::vector<uint8_t>& buffer) {
    for (auto& byte : buffer) {
        byte ^= XARY_XOR_KEY;
    }
}

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
            std::cout << "[+] Encoding: " << options.inputFile << " -> " << outPath << "\n";

            core::Stream reader(options.inputFile, 64 * 1024);
            if (!reader.isOpen()) {
                std::cerr << "❌ Error: Could not open source file '" << options.inputFile << "'\n";
                return 1;
            }

            core::StreamWriter writer(outPath);
            if (!writer.isOpen()) {
                std::cerr << "❌ Error: Could not create output file '" << outPath << "'\n";
                return 1;
            }

            std::vector<uint8_t> buffer;
            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            while (std::size_t bytesRead = reader.readChunk(buffer)) {
                transformBuffer(buffer);
                if (!writer.writeChunk(buffer)) {
                    std::cerr << "❌ Error: Failed to write output chunk.\n";
                    return 1;
                }
                totalBytes += bytesRead;
                chunks++;
            }

            std::cout << "✔ Encoded " << totalBytes << " bytes across " << chunks << " chunk(s).\n";
            return 0;
        }

        case cli::Mode::Decode: {
            std::string outPath = options.outputFile.empty() ? (options.inputFile + ".out") : options.outputFile;
            std::cout << "[+] Decoding: " << options.inputFile << " -> " << outPath << "\n";

            core::Stream reader(options.inputFile, 64 * 1024);
            if (!reader.isOpen()) {
                std::cerr << "❌ Error: Could not open source file '" << options.inputFile << "'\n";
                return 1;
            }

            core::StreamWriter writer(outPath);
            if (!writer.isOpen()) {
                std::cerr << "❌ Error: Could not create output file '" << outPath << "'\n";
                return 1;
            }

            std::vector<uint8_t> buffer;
            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            while (std::size_t bytesRead = reader.readChunk(buffer)) {
                transformBuffer(buffer); // XOR is symmetric: XORing twice restores original bytes
                if (!writer.writeChunk(buffer)) {
                    std::cerr << "❌ Error: Failed to write decoded chunk.\n";
                    return 1;
                }
                totalBytes += bytesRead;
                chunks++;
            }

            std::cout << "✔ Decoded " << totalBytes << " bytes across " << chunks << " chunk(s).\n";
            return 0;
        }

        case cli::Mode::None:
        default:
            cli::ArgumentParser::printHelp();
            return 0;
    }
}