#pragma once

#include <cstdint>

#include "isp_block.hpp"

struct DpcKernelParams
{
    int32_t diff_threshold;
};

class DpcBlock : public IspBlock
{
  public:
    explicit DpcBlock(const IspConfig& cfg, cudaStream_t stream = 0);

    void execute(PipelineData& data) override;
    const char*
    name() const override
    {
        return "dpc";
    }

  private:
    DpcKernelParams params_;
};
