#include "exclusive_output.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::string next_token(std::istream& in) {
    std::string token;
    while (in >> token) {
        if (token[0] == '#') { std::string rest; std::getline(in, rest); continue; }
        return token;
    }
    return "";
}

long long integer(const std::string& text, const std::string& field) {
    if (text.empty()) throw std::runtime_error("missing " + field);
    std::size_t end = 0;
    long long value;
    try { value = std::stoll(text, &end); }
    catch (const std::exception&) { throw std::runtime_error("invalid " + field); }
    if (end != text.size()) throw std::runtime_error("invalid " + field);
    return value;
}

void run(const fs::path& input, const fs::path& output) {
    std::ifstream in(input, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open input image");
    if (next_token(in) != "P2") throw std::runtime_error("expected ASCII P2 PGM image");
    const long long width = integer(next_token(in), "width");
    const long long height = integer(next_token(in), "height");
    const long long maximum = integer(next_token(in), "maximum pixel value");
    if (width < 1 || height < 1 || width > 10000 || height > 10000 || width * height > 10000000)
        throw std::runtime_error("image dimensions must fit within 10 million pixels");
    if (maximum < 1 || maximum > 255) throw std::runtime_error("maximum pixel value must be 1..255");
    std::vector<int> pixels(static_cast<std::size_t>(width * height));
    for (int& pixel : pixels) {
        const long long value = integer(next_token(in), "pixel");
        if (value < 0 || value > maximum) throw std::runtime_error("pixel outside declared range");
        pixel = static_cast<int>(value);
    }
    if (!next_token(in).empty()) throw std::runtime_error("extra pixel data");
    std::vector<int> edges(pixels.size(), 0);
    auto at = [&](long long x, long long y) { return pixels[static_cast<std::size_t>(y * width + x)]; };
    for (long long y = 1; y + 1 < height; ++y) {
        for (long long x = 1; x + 1 < width; ++x) {
            const int gx = -at(x-1,y-1) + at(x+1,y-1) - 2*at(x-1,y) + 2*at(x+1,y) - at(x-1,y+1) + at(x+1,y+1);
            const int gy = -at(x-1,y-1) - 2*at(x,y-1) - at(x+1,y-1) + at(x-1,y+1) + 2*at(x,y+1) + at(x+1,y+1);
            const int magnitude = static_cast<int>(std::sqrt(static_cast<double>(gx*gx + gy*gy)));
            edges[static_cast<std::size_t>(y * width + x)] = std::min(255, magnitude);
        }
    }
    ExclusiveOutput file(output);
    std::ostream& out = file.stream();
    out << "P2\n" << width << ' ' << height << "\n255\n";
    for (long long y = 0; y < height; ++y) {
        for (long long x = 0; x < width; ++x) {
            if (x) out << ' ';
            out << edges[static_cast<std::size_t>(y * width + x)];
        }
        out << '\n';
    }
    file.publish();
    std::cout << "pixels=" << pixels.size() << "\n";
}

int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::runtime_error("usage: edges <input.pgm> <output.pgm>");
        run(argv[1], argv[2]);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
