#pragma once

#include <cstdint>

#include "isp_block.hpp"

struct BlcKernelParams
{
    int32_t bl_r, bl_gr, bl_gb, bl_b;
    int32_t alpha, beta;  // x1024 fixed-point
    uint16_t sat_value;
    uint8_t r_row, r_col;    // position of R  in the 2×2 Bayer tile
    uint8_t gr_row, gr_col;  // position of Gr in the 2×2 Bayer tile
    uint8_t gb_row, gb_col;  // position of Gb in the 2×2 Bayer tile
    uint8_t b_row, b_col;    // position of B  in the 2×2 Bayer tile
};

class BlcBlock : public IspBlock
{
  public:
    explicit BlcBlock(const IspConfig& cfg, cudaStream_t stream = 0);

    void execute(PipelineData& data) override;
    const char*
    name() const override
    {
        return "blc";
    }

  private:
    BlcKernelParams params_;
};
