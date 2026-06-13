#pragma once
#include "isp_block.hpp"

struct AafKernelParams
{
};

class AafBlock : public IspBlock
{
  public:
    explicit AafBlock(const IspConfig& cfg, cudaStream_t stream = 0);

    void execute(PipelineData& data) override;
    const char*
    name() const override
    {
        return "aaf";
    }

  private:
    AafKernelParams params_;
};
