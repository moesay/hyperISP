#pragma once

#include <cstdint>
#include <fstream>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>

#include <frame.hpp>

inline PitchedFrame<uint16_t>
load_raw(const std::string& path, uint32_t width, uint32_t height)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
    {
        throw std::runtime_error("load_raw: cannot open " + path);
    }

    f.seekg(0, std::ios::end);
    const auto file_size = static_cast<size_t>(f.tellg());
    f.seekg(0, std::ios::beg);

    const size_t expected = static_cast<size_t>(width) * height * sizeof(uint16_t);
    if (file_size != expected)
    {
        throw std::runtime_error("load_raw: file is " + std::to_string(file_size) + " bytes but " +
                                 std::to_string(width) + "x" + std::to_string(height) +
                                 " requires " + std::to_string(expected));
    }

    PitchedFrame<uint16_t> frame(width, height);
    std::vector<uint16_t> row(width);

    for (uint32_t y = 0; y < height; ++y)
    {
        if (!f.read(reinterpret_cast<char*>(row.data()),
                    static_cast<std::streamsize>(width * sizeof(uint16_t))))
        {
            throw std::runtime_error("load_raw: unexpected EOF at row " + std::to_string(y) +
                                     " in '" + path + "'");
        }
        for (uint32_t x = 0; x < width; ++x)
        {
            frame.at(y, x) = row[x];
        }
    }

    return frame;
}

inline void
save_raw(PitchedFrame<uint16_t>& frame, const std::string& path)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
    {
        throw std::runtime_error("save_raw: cannot open " + path + " for writing");
    }

    std::vector<uint16_t> row(frame.width());
    for (uint32_t y = 0; y < frame.height(); ++y)
    {
        for (uint32_t x = 0; x < frame.width(); ++x)
        {
            row[x] = frame.at(y, x);
        }
        if (!f.write(reinterpret_cast<const char*>(row.data()),
                     static_cast<std::streamsize>(frame.width() * sizeof(uint16_t))))
        {
            throw std::runtime_error("save_raw: write error at row " + std::to_string(y));
        }
    }
    std::println("Saved        : {}", path);
}

template <typename T>
inline void
save_rgb(PitchedFrame<T>& frame, const std::string& path)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
    {
        throw std::runtime_error("save_rgb: cannot open " + path + " for writing");
    }

    const uint32_t channels = frame.channels();
    std::vector<T> row(static_cast<size_t>(frame.width()) * channels);

    for (uint32_t y = 0; y < frame.height(); ++y)
    {
        for (uint32_t x = 0; x < frame.width(); ++x)
        {
            for (uint32_t c = 0; c < channels; ++c)
            {
                row[x * channels + c] = frame.at(y, x, c);
            }
        }
        if (!f.write(reinterpret_cast<const char*>(row.data()),
                     static_cast<std::streamsize>(row.size() * sizeof(T))))
        {
            throw std::runtime_error("save_rgb: write error at row " + std::to_string(y));
        }
    }
    std::println("Saved        : {}", path);
}
