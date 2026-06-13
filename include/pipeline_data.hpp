// File: pipeline_data.hpp
// Description: Frame buffers shared across the ISP pipeline
// Author: Mohamed ElKafafy (m.elsayed4420@gmail.com)
// Licensed under the GNU General Public License v3.0 (GPL-3.0)

#pragma once

#include <memory>

#include "frame.hpp"

struct PipelineData
{
    std::unique_ptr<PitchedFrame<uint16_t>> bayer;    // raw Bayer, 1ch
    std::unique_ptr<PitchedFrame<uint16_t>> rgb_hdr;  // demosaiced RGB, 3ch, uint16
    std::unique_ptr<PitchedFrame<uint8_t>> rgb_sdr;   // gamma-compressed RGB, 3ch, uint8
    std::unique_ptr<PitchedFrame<uint8_t>> ycbcr;     // YCbCr, 3ch, uint8
};
