// File: isp_block.hpp
// Description: Base interface for ISP pipeline blocks
// Author: Mohamed ElKafafy (m.elsayed4420@gmail.com)
// Licensed under the GNU General Public License v3.0 (GPL-3.0)

#pragma once

#include <cuda_runtime.h>

#include "config.hpp"
#include "pipeline_data.hpp"

class IspBlock
{
  public:
    explicit IspBlock(const IspConfig& cfg, cudaStream_t stream = 0) : cfg_(cfg), stream_(stream) {}

    virtual ~IspBlock() = default;

    IspBlock(const IspBlock&) = delete;
    IspBlock& operator=(const IspBlock&) = delete;

    /*
       TODO:
       currently, execute() will throw an exception in there is no frame to process.
       "Is it better to just skip the frame?" I'm not sure about the answer rn so i will leave this
       here
    */
    virtual void execute(PipelineData& data) = 0;
    virtual const char* name() const = 0;

  protected:
    const IspConfig& cfg_;
    cudaStream_t stream_;
};
