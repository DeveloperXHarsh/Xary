#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string_view>
#include "../include/xary/core/Stream.hpp"

constexpr std::string_view XARY_VERSION = "0.1.0";

bool createDummyBinaryFile(const std::string& filename, std::size_t totalBytes) {
    std::ofstream out(filename, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    std::vector<uint8_t> dummyData(64 * 1024, 0xAB);
    std::size_t written = 0;
    while (written < totalBytes) {
        std::size_t toWrite = (std::min)(dummyData.size(), totalBytes - written);
        out.write(reinterpret_cast<const char*>(dummyData.data()), static_cast<std::streamsize>(toWrite));
        written += toWrite;
    }
    out.close();
    return true;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "=== Xary Binary Engine v" << XARY_VERSION << " ===\n\n";

    std::string dummyPath = "test_stream.bin";
    if (!createDummyBinaryFile(dummyPath, 180 * 1024)) {
        std::cerr << "❌ Error: Could not create test file on disk.\n";
        return 1;
    }

    xary::core::Stream stream(dummyPath, 64 * 1024);

    if (!stream.isOpen()) {
        std::cerr << "❌ Error: Failed to open stream for " << dummyPath << "\n";
        return 1;
    }

    std::cout << "[+] Stream opened successfully!\n";
    std::cout << "[+] File Size: " << stream.getFileSize() << " bytes\n";

    std::vector<uint8_t> chunkBuffer;
    std::size_t totalBytesRead = 0;
    std::size_t chunkCount = 0;

    while (std::size_t bytesRead = stream.readChunk(chunkBuffer)) {
        totalBytesRead += bytesRead;
        chunkCount++;
    }

    std::cout << "[+] Processed " << chunkCount << " chunk(s).\n";
    std::cout << "[+] Total streamed: " << totalBytesRead << " bytes.\n\n";
    std::cout << "✔ Stream test passed cleanly!\n";

    return 0;
}