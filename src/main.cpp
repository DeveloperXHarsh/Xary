#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>
#include "../include/xary/core/Stream.hpp"
#include "../include/xary/core/StreamWriter.hpp"
#include "../include/xary/cli/ArgumentParser.hpp"

using namespace xary;

// 1. Non-printable 8-byte Raw Binary Header (Scrambled & unreadable in Notepad)
constexpr std::array<uint8_t, 8> XARY_BINARY_MAGIC = {
    0x8F, 0x1E, 0xAA, 0x4D, 0x9C, 0x05, 0xF3, 0x72
};

// 2. Secret 16-Byte Master Key Matrix
constexpr std::array<uint8_t, 16> XARY_MASTER_KEY = {
    0xD4, 0x2A, 0x8F, 0x11, 0x39, 0x7B, 0x9C, 0xE7,
    0x51, 0x64, 0xB2, 0x08, 0xF7, 0xAE, 0x19, 0x3C
};

// ---------------------------------------------------------------------------
// Proprietary Bit-Rotation & Dynamic Key Engine
// ---------------------------------------------------------------------------

// Bitwise Left-Rotate 8-bit byte
inline uint8_t rotl8(uint8_t value, unsigned int count) {
    return static_cast<uint8_t>((value << count) | (value >> (8 - count)));
}

// Bitwise Right-Rotate 8-bit byte
inline uint8_t rotr8(uint8_t value, unsigned int count) {
    return static_cast<uint8_t>((value >> count) | (value << (8 - count)));
}

// Proprietary Encoding Cipher (Only Xary's mathematical inverse can reverse this)
void encodeBuffer(std::vector<uint8_t>& buffer, std::size_t globalStreamOffset) {
    for (std::size_t i = 0; i < buffer.size(); ++i) {
        std::size_t pos = globalStreamOffset + i;
        uint8_t key = XARY_MASTER_KEY[pos % XARY_MASTER_KEY.size()] ^ static_cast<uint8_t>(pos & 0xFF);
        
        uint8_t byte = buffer[i];
        byte ^= key;                   // Step 1: Dynamic XOR
        byte = rotl8(byte, 3);          // Step 2: 3-Bit Left Rotation
        byte ^= 0xA5;                  // Step 3: Secondary Salt Mask
        buffer[i] = byte;
    }
}

// Proprietary Decoding Cipher (Exact Mathematical Inverse)
void decodeBuffer(std::vector<uint8_t>& buffer, std::size_t globalStreamOffset) {
    for (std::size_t i = 0; i < buffer.size(); ++i) {
        std::size_t pos = globalStreamOffset + i;
        uint8_t key = XARY_MASTER_KEY[pos % XARY_MASTER_KEY.size()] ^ static_cast<uint8_t>(pos & 0xFF);
        
        uint8_t byte = buffer[i];
        byte ^= 0xA5;                  // Inverse Step 3: Remove Salt
        byte = rotr8(byte, 3);          // Inverse Step 2: 3-Bit Right Rotation
        byte ^= key;                   // Inverse Step 1: Remove Dynamic XOR
        buffer[i] = byte;
    }
}

// ---------------------------------------------------------------------------
// Engine Main Entry Point
// ---------------------------------------------------------------------------

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

            // Write invisible 8-byte binary magic header
            writer.writeChunk(XARY_BINARY_MAGIC.data(), XARY_BINARY_MAGIC.size());

            std::vector<uint8_t> buffer;
            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            while (std::size_t bytesRead = reader.readChunk(buffer)) {
                encodeBuffer(buffer, totalBytes);
                if (!writer.writeChunk(buffer)) {
                    std::cerr << "❌ Error: Failed to write encoded chunk.\n";
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
            std::cout << "[+] Processing/Decoding: " << options.inputFile << " -> " << outPath << "\n";

            core::Stream reader(options.inputFile, 64 * 1024);
            if (!reader.isOpen()) {
                std::cerr << "❌ Error: Could not open target file '" << options.inputFile << "'\n";
                return 1;
            }

            // Inspect header
            std::vector<uint8_t> firstChunk;
            std::size_t bytesRead = reader.readChunk(firstChunk);

            if (bytesRead < XARY_BINARY_MAGIC.size()) {
                std::cerr << "❌ Error: File is too small to process.\n";
                return 1;
            }

            // Validate Xary Magic Signature
            bool isXaryFile = true;
            for (std::size_t i = 0; i < XARY_BINARY_MAGIC.size(); ++i) {
                if (firstChunk[i] != XARY_BINARY_MAGIC[i]) {
                    isXaryFile = false;
                    break;
                }
            }

            core::StreamWriter writer(outPath);
            if (!writer.isOpen()) {
                std::cerr << "❌ Error: Could not create destination file '" << outPath << "'\n";
                return 1;
            }

            std::vector<uint8_t> buffer;
            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            if (isXaryFile) {
                std::cout << "✔ Xary Encrypted Signature recognized. Decrypting...\n";

                // Process first chunk payload (after 8-byte binary magic header)
                if (firstChunk.size() > XARY_BINARY_MAGIC.size()) {
                    std::vector<uint8_t> payload(
                        firstChunk.begin() + XARY_BINARY_MAGIC.size(),
                        firstChunk.end()
                    );
                    decodeBuffer(payload, 0);
                    writer.writeChunk(payload);
                    totalBytes += payload.size();
                    chunks++;
                }

                // Stream remaining 64 KB blocks
                while (std::size_t read = reader.readChunk(buffer)) {
                    decodeBuffer(buffer, totalBytes);
                    writer.writeChunk(buffer);
                    totalBytes += read;
                    chunks++;
                }

                std::cout << "✔ Decoded " << totalBytes << " bytes across " << chunks << " chunk(s).\n";
            } else {
                std::cout << "ℹ Raw binary stream detected (Non-Xary format). Streaming raw pass-through...\n";

                // Process raw binary file stream (Xary can read/process any binary file)
                writer.writeChunk(firstChunk);
                totalBytes += firstChunk.size();
                chunks++;

                while (std::size_t read = reader.readChunk(buffer)) {
                    writer.writeChunk(buffer);
                    totalBytes += read;
                    chunks++;
                }

                std::cout << "✔ Streamed " << totalBytes << " raw binary bytes successfully.\n";
            }

            return 0;
        }

        case cli::Mode::None:
        default:
            cli::ArgumentParser::printHelp();
            return 0;
    }
}