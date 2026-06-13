// File: config.hpp
// Description: ISP pipeline configuration types and TOML loader
// Author: Mohamed ElKafafy (m.elsayed4420@gmail.com)
// Licensed under the GNU General Public License v3.0 (GPL-3.0)

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <toml++/toml.hpp>

struct HardwareConfig
{
    uint32_t raw_width;
    uint32_t raw_height;
    uint32_t raw_bit_depth;
    std::string bayer_pattern;
};

struct IspConfig
{
    std::unordered_map<std::string, bool> block_enable_status;
    HardwareConfig hardware;

    // the row toml is kept to check for the enabled modules
    toml::table raw;

    // for aaf, csc, this function will return a nullptr because they have
    // no params. Otherwise, it will return the module params as a table
    const toml::table* block_params(const std::string& name) const;

    static IspConfig load(const std::string& path);
};
