#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>
#include <filesystem>
#include "../include/xary/core/Stream.hpp"
#include "../include/xary/core/StreamWriter.hpp"
#include "../include/xary/core/FileTypeDetector.hpp"
#include "../include/xary/cli/ArgumentParser.hpp"

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Main Pipeline Launcher (main.cpp)
 * Description : High-performance binary streaming tool featuring stream-based
 *               cipher transformation and magic byte signature inspection.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

using namespace xary;

constexpr std::array<uint8_t, 8> XARY_BINARY_MAGIC = {
    0x8F, 0x1E, 0xAA, 0x4D, 0x9C, 0x05, 0xF3, 0x72
};

constexpr std::array<uint8_t, 16> XARY_MASTER_KEY = {
    0xD4, 0x2A, 0x8F, 0x11, 0x39, 0x7B, 0x9C, 0xE7,
    0x51, 0x64, 0xB2, 0x08, 0xF7, 0xAE, 0x19, 0x3C
};

inline uint8_t rotl8(uint8_t value, unsigned int count) noexcept {
    return static_cast<uint8_t>((value << count) | (value >> (8 - count)));
}

inline uint8_t rotr8(uint8_t value, unsigned int count) noexcept {
    return static_cast<uint8_t>((value >> count) | (value << (8 - count)));
}

void encodeBufferOptimized(std::vector<uint8_t>& buffer, std::size_t globalStreamOffset) noexcept {
    const std::size_t size = buffer.size();
    uint8_t* ptr = buffer.data();
    std::size_t i = 0;

    for (; i + 15 < size; i += 16) {
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC unroll 16
#endif
        for (std::size_t k = 0; k < 16; ++k) {
            std::size_t pos = globalStreamOffset + i + k;
            uint8_t key = XARY_MASTER_KEY[pos % 16] ^ static_cast<uint8_t>(pos & 0xFF);
            uint8_t byte = ptr[i + k] ^ key;
            byte = rotl8(byte, 3);
            ptr[i + k] = byte ^ 0xA5;
        }
    }

    for (; i < size; ++i) {
        std::size_t pos = globalStreamOffset + i;
        uint8_t key = XARY_MASTER_KEY[pos % 16] ^ static_cast<uint8_t>(pos & 0xFF);
        uint8_t byte = ptr[i] ^ key;
        byte = rotl8(byte, 3);
        ptr[i] = byte ^ 0xA5;
    }
}

void decodeBufferOptimized(std::vector<uint8_t>& buffer, std::size_t globalStreamOffset) noexcept {
    const std::size_t size = buffer.size();
    uint8_t* ptr = buffer.data();
    std::size_t i = 0;

    for (; i + 15 < size; i += 16) {
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC unroll 16
#endif
        for (std::size_t k = 0; k < 16; ++k) {
            std::size_t pos = globalStreamOffset + i + k;
            uint8_t key = XARY_MASTER_KEY[pos % 16] ^ static_cast<uint8_t>(pos & 0xFF);
            uint8_t byte = ptr[i + k] ^ 0xA5;
            byte = rotr8(byte, 3);
            ptr[i + k] = byte ^ key;
        }
    }

    for (; i < size; ++i) {
        std::size_t pos = globalStreamOffset + i;
        uint8_t key = XARY_MASTER_KEY[pos % 16] ^ static_cast<uint8_t>(pos & 0xFF);
        uint8_t byte = ptr[i] ^ 0xA5;
        byte = rotr8(byte, 3);
        ptr[i] = byte ^ key;
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

        case cli::Mode::Inspect: {
            std::cout << "[🔍] Inspecting binary signature: " << options.inputFile.string() << "\n\n";

            core::Stream reader(options.inputFile, 64 * 1024);
            if (!reader.isOpen()) {
                std::cerr << "❌ Error: Could not open target file '" << options.inputFile.string() << "'\n";
                return 1;
            }

            std::vector<uint8_t> headerChunk;
            std::size_t bytesRead = reader.readChunk(headerChunk);

            if (bytesRead == 0) {
                std::cerr << "❌ Error: File is empty.\n";
                return 1;
            }

            core::BufferView view(headerChunk);
            core::FileTypeInfo info = core::FileTypeDetector::detect(view);

            std::cout << "  File Description : " << info.description << "\n";
            std::cout << "  MIME Type        : " << info.mimeType << "\n";
            std::cout << "  Detected Ext     : " << info.expectedExtension << "\n";

            std::string currentFile = options.inputFile.string();
            if (info.isKnown && currentFile.find(info.expectedExtension) == std::string::npos) {
                std::cout << "\n⚠️  WARNING: File extension appears corrupt or mismatched!\n";
                std::cout << "👉 Suggested Fix : Rename file to end with '" << info.expectedExtension << "'\n";
            } else if (info.isKnown) {
                std::cout << "\n✅ File extension matches header signature perfectly!\n";
            }

            return 0;
        }

        case cli::Mode::Encode: {
            std::filesystem::path outPath = options.outputFile.empty()
                ? std::filesystem::path(options.inputFile.string() + ".xary")
                : options.outputFile;

            std::cout << "[+] Encoding: " << options.inputFile.string() << " -> " << outPath.string() << "\n";

            core::Stream reader(options.inputFile, 64 * 1024);
            if (!reader.isOpen()) {
                std::cerr << "❌ Error: Could not open source file '" << options.inputFile.string() << "'\n";
                return 1;
            }

            core::StreamWriter writer(outPath);
            if (!writer.isOpen()) {
                std::cerr << "❌ Error: Could not create output file '" << outPath.string() << "'\n";
                return 1;
            }

            if (!writer.writeChunk(XARY_BINARY_MAGIC.data(), XARY_BINARY_MAGIC.size())) {
                std::cerr << "❌ Error: Failed to write magic header sequence.\n";
                return 1;
            }

            std::vector<uint8_t> buffer;
            buffer.reserve(64 * 1024);

            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            while (std::size_t bytesRead = reader.readChunk(buffer)) {
                encodeBufferOptimized(buffer, totalBytes);
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
            std::filesystem::path outPath = options.outputFile.empty()
                ? std::filesystem::path(options.inputFile.string() + ".out")
                : options.outputFile;

            std::cout << "[+] Decoding/Processing: " << options.inputFile.string() << " -> " << outPath.string() << "\n";

            core::Stream reader(options.inputFile, 64 * 1024);
            if (!reader.isOpen()) {
                std::cerr << "❌ Error: Could not open target file '" << options.inputFile.string() << "'\n";
                return 1;
            }

            std::vector<uint8_t> firstChunk;
            firstChunk.reserve(64 * 1024);
            std::size_t bytesRead = reader.readChunk(firstChunk);

            if (bytesRead < XARY_BINARY_MAGIC.size()) {
                std::cerr << "❌ Error: File is too small to process.\n";
                return 1;
            }

            bool isXaryFile = true;
            for (std::size_t i = 0; i < XARY_BINARY_MAGIC.size(); ++i) {
                if (firstChunk[i] != XARY_BINARY_MAGIC[i]) {
                    isXaryFile = false;
                    break;
                }
            }

            core::StreamWriter writer(outPath);
            if (!writer.isOpen()) {
                std::cerr << "❌ Error: Could not create destination file '" << outPath.string() << "'\n";
                return 1;
            }

            std::vector<uint8_t> buffer;
            buffer.reserve(64 * 1024);

            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            if (isXaryFile) {
                std::cout << "✔ Xary Encrypted Signature recognized. Decrypting...\n";

                if (firstChunk.size() > XARY_BINARY_MAGIC.size()) {
                    std::vector<uint8_t> payload(
                        firstChunk.begin() + XARY_BINARY_MAGIC.size(),
                        firstChunk.end()
                    );
                    decodeBufferOptimized(payload, 0);
                    if (!writer.writeChunk(payload)) {
                        std::cerr << "❌ Error: Failed to write output payload.\n";
                        return 1;
                    }
                    totalBytes += payload.size();
                    chunks++;
                }

                while (std::size_t read = reader.readChunk(buffer)) {
                    decodeBufferOptimized(buffer, totalBytes);
                    if (!writer.writeChunk(buffer)) {
                        std::cerr << "❌ Error: Failed to write output chunk.\n";
                        return 1;
                    }
                    totalBytes += read;
                    chunks++;
                }

                std::cout << "✔ Decoded " << totalBytes << " bytes across " << chunks << " chunk(s).\n";
            } else {
                std::cout << "ℹ Raw binary stream detected. Pass-through streaming...\n";
                if (!writer.writeChunk(firstChunk)) {
                    std::cerr << "❌ Error: Failed to write stream chunk.\n";
                    return 1;
                }
                totalBytes += firstChunk.size();
                chunks++;

                while (std::size_t read = reader.readChunk(buffer)) {
                    if (!writer.writeChunk(buffer)) {
                        std::cerr << "❌ Error: Failed to write stream chunk.\n";
                        return 1;
                    }
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