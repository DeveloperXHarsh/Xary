#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <chrono>
#include "../include/xary/core/Stream.hpp"
#include "../include/xary/core/StreamWriter.hpp"
#include "../include/xary/core/FileTypeDetector.hpp"
#include "../include/xary/core/Archiver.hpp"
#include "../include/xary/cli/ArgumentParser.hpp"

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Main Pipeline Launcher (main.cpp)
 * Description : High-performance binary streaming tool featuring stream-based
 *               cipher transformation, magic signature inspection, and stealth
 *               archiving (--sec) with strict 64 KB memory bounds.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

using namespace xary;

// Special 8-byte signature placed at the start of encoded files.
// This helps Xary identify files that were encoded by this engine.
constexpr std::array<uint8_t, 8> XARY_BINARY_MAGIC = {
    0x8F, 0x1E, 0xAA, 0x4D, 0x9C, 0x05, 0xF3, 0x72
};

// A fixed secret key pattern (16 bytes) used to scramble and unscramble data.
constexpr std::array<uint8_t, 16> XARY_MASTER_KEY = {
    0xD4, 0x2A, 0x8F, 0x11, 0x39, 0x7B, 0x9C, 0xE7,
    0x51, 0x64, 0xB2, 0x08, 0xF7, 0xAE, 0x19, 0x3C
};

// Rotate bits to the left (Bitwise Left Rotation).
// Moves bits to the left, and bits that fall off the left end come back on the right end.
inline uint8_t rotl8(uint8_t value, unsigned int count) noexcept {
    return static_cast<uint8_t>((value << count) | (value >> (8 - count)));
}

// Rotate bits to the right (Bitwise Right Rotation).
// Moves bits to the right, and bits that fall off the right end come back on the left end.
inline uint8_t rotr8(uint8_t value, unsigned int count) noexcept {
    return static_cast<uint8_t>((value >> count) | (value << (8 - count)));
}

// Scramble (encrypt) a memory buffer of bytes.
void encodeBufferOptimized(std::vector<uint8_t>& buffer, std::size_t globalStreamOffset) noexcept {
    const std::size_t size = buffer.size();
    uint8_t* ptr = buffer.data();
    std::size_t i = 0;

    // Fast Loop: Process 16 bytes at a time in batch for higher CPU performance.
    for (; i + 15 < size; i += 16) {
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC unroll 16 // Tells compiler to expand loop for extra speed
#endif
        for (std::size_t k = 0; k < 16; ++k) {
            std::size_t pos = globalStreamOffset + i + k;
            
            // 1. Calculate a dynamic key byte based on position and master key
            uint8_t key = XARY_MASTER_KEY[pos % 16] ^ static_cast<uint8_t>(pos & 0xFF);
            
            // 2. Scramble byte using XOR, bit rotation, and a mask (0xA5)
            uint8_t byte = ptr[i + k] ^ key;
            byte = rotl8(byte, 3);
            ptr[i + k] = byte ^ 0xA5;
        }
    }

    // Leftover Loop: Scramble any remaining bytes if total size is not a multiple of 16.
    for (; i < size; ++i) {
        std::size_t pos = globalStreamOffset + i;
        uint8_t key = XARY_MASTER_KEY[pos % 16] ^ static_cast<uint8_t>(pos & 0xFF);
        uint8_t byte = ptr[i] ^ key;
        byte = rotl8(byte, 3);
        ptr[i] = byte ^ 0xA5;
    }
}

// Unscramble (decrypt) a memory buffer of bytes.
void decodeBufferOptimized(std::vector<uint8_t>& buffer, std::size_t globalStreamOffset) noexcept {
    const std::size_t size = buffer.size();
    uint8_t* ptr = buffer.data();
    std::size_t i = 0;

    // Fast Loop: Process 16 bytes at a time in exact reverse order of encoding.
    for (; i + 15 < size; i += 16) {
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC unroll 16
#endif
        for (std::size_t k = 0; k < 16; ++k) {
            std::size_t pos = globalStreamOffset + i + k;
            uint8_t key = XARY_MASTER_KEY[pos % 16] ^ static_cast<uint8_t>(pos & 0xFF);
            
            // Reverses the encoding steps: XOR mask -> rotate right -> XOR key
            uint8_t byte = ptr[i + k] ^ 0xA5;
            byte = rotr8(byte, 3);
            ptr[i + k] = byte ^ key;
        }
    }

    // Leftover Loop: Unscramble any remaining bytes.
    for (; i < size; ++i) {
        std::size_t pos = globalStreamOffset + i;
        uint8_t key = XARY_MASTER_KEY[pos % 16] ^ static_cast<uint8_t>(pos & 0xFF);
        uint8_t byte = ptr[i] ^ 0xA5;
        byte = rotr8(byte, 3);
        ptr[i] = byte ^ key;
    }
}

int main(int argc, char* argv[]) {
    // Read command line arguments typed by user in terminal (e.g., xary -i sample.png)
    cli::Options options = cli::ArgumentParser::parse(argc, argv);

    // If options are invalid or incorrect flags were passed, show error and exit
    if (!options.isValid) {
        std::cerr << "❌ Error: " << options.errorMessage << "\n\n";
        cli::ArgumentParser::printHelp();
        return 1;
    }

    // Determine what action to take based on user command
    switch (options.mode) {
        case cli::Mode::Help:
            cli::ArgumentParser::printHelp();
            return 0;

        case cli::Mode::Version:
            cli::ArgumentParser::printVersion();
            return 0;

        // MODE 1: Inspect File Signature (-i)
        case cli::Mode::Inspect: {
            std::cout << "[🔍] Inspecting binary signature: " << options.inputFile.string() << "\n\n";

            // Open file using 64 KB chunk reader
            core::Stream reader(options.inputFile, 64 * 1024);
            if (!reader.isOpen()) {
                std::cerr << "❌ Error: Could not open target file '" << options.inputFile.string() << "'\n";
                return 1;
            }

            // Read first 64 KB chunk to inspect header magic bytes
            std::vector<uint8_t> headerChunk;
            std::size_t bytesRead = reader.readChunk(headerChunk);

            if (bytesRead == 0) {
                std::cerr << "❌ Error: File is empty.\n";
                return 1;
            }

            // Check signature pattern against internal database
            core::BufferView view(headerChunk);
            core::FileTypeInfo info = core::FileTypeDetector::detect(view);

            std::cout << "  File Description : " << info.description << "\n";
            std::cout << "  MIME Type        : " << info.mimeType << "\n";
            std::cout << "  Detected Ext     : " << info.expectedExtension << "\n";

            // Warn user if file extension on disk does not match real file signature
            std::string currentFile = options.inputFile.string();
            if (info.isKnown && currentFile.find(info.expectedExtension) == std::string::npos) {
                std::cout << "\n⚠️  WARNING: File extension appears corrupt or mismatched!\n";
                std::cout << "👉 Suggested Fix : Rename file to end with '" << info.expectedExtension << "'\n";
            } else if (info.isKnown) {
                std::cout << "\n✅ File extension matches header signature perfectly!\n";
            }

            return 0;
        }

        // MODE 2: Encode File (-e)
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

            // Step 1: Write special Xary magic header to output file
            if (!writer.writeChunk(XARY_BINARY_MAGIC.data(), XARY_BINARY_MAGIC.size())) {
                std::cerr << "❌ Error: Failed to write magic header sequence.\n";
                return 1;
            }

            std::vector<uint8_t> buffer;
            buffer.reserve(64 * 1024);

            std::size_t totalBytes = 0;
            std::size_t chunks = 0;

            // Step 2: Read file in 64 KB pieces, encode each piece, and write to output file
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

        // MODE 3: Decode File (-d)
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

            // Check if input file starts with Xary magic header
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

            // Scenario A: File is an encoded Xary binary -> Decrypt data chunks
            if (isXaryFile) {
                std::cout << "✔ Xary Encrypted Signature recognized. Decrypting...\n";

                // Strip magic header and decode remaining payload in first chunk
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

                // Decode all remaining 64 KB chunks
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
            } 
            // Scenario B: File is raw binary -> Stream data directly without modification (Pass-through)
            else {
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

        // MODE 4: Pack Folder/File into Archive (-p)
        case cli::Mode::Pack: {
            std::filesystem::path outPath = options.outputFile.empty()
                ? std::filesystem::path(options.inputFile.string() + ".xary")
                : options.outputFile;

            std::cout << "[📦] Packing Target: " << options.inputFile.string() << " -> " << outPath.string() << "\n";
            if (options.secureMode) {
                std::cout << "  🔒 Mode: SECURE (--sec) | High-Entropy Stealth Header Active\n";
            }

            // Measure execution time using high-precision timer
            auto startTime = std::chrono::high_resolution_clock::now();
            core::Archiver archiver;

            // Pack target directory structure
            if (archiver.pack(options.inputFile, outPath, options.secureMode, options.key)) {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                std::cout << "✔ Successfully packed archive in " << duration << " ms!\n";
                return 0;
            } else {
                std::cerr << "❌ Error: Failed to generate archive package.\n";
                return 1;
            }
        }

        // MODE 5: Unpack Archive into Directory (-u)
        case cli::Mode::Unpack: {
            std::filesystem::path outDir = options.outputFile.empty()
                ? std::filesystem::path("./extracted")
                : options.outputFile;

            std::cout << "[🔓] Unpacking Archive: " << options.inputFile.string() << " -> " << outDir.string() << "\n";

            // Measure execution time using high-precision timer
            auto startTime = std::chrono::high_resolution_clock::now();
            core::Archiver archiver;

            // Extract archive and rebuild original directory tree
            if (archiver.unpack(options.inputFile, outDir, options.key)) {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                std::cout << "✔ Successfully unpacked archive in " << duration << " ms!\n";
                return 0;
            } else {
                std::cerr << "❌ Error: Failed to unpack archive. File may be corrupted or key is invalid.\n";
                return 1;
            }
        }

        case cli::Mode::None:
        default:
            cli::ArgumentParser::printHelp();
            return 0;
    }
}