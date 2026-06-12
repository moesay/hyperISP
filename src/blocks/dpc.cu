#include <stdexcept>

#include "blocks/dpc.hpp"

namespace
{

DpcKernelParams
make_params(const IspConfig& cfg)
{
    const toml::table* t = cfg.block_params("dpc");
    if (!t)
    {
        throw std::runtime_error("DpcBlock: missing [dpc] config section");
    }

    DpcKernelParams kernel_params{};
    kernel_params.diff_threshold = (*t)["diff_threshold"].value<int64_t>().value_or(0);
    return kernel_params;
}

}  // namespace

/*
   TODO:
   writes go to a separate buffer because a thread's neighbor might be a dead pixels
   being corrected by other thread. Lockstep execution should take care of this
   but it's a deprecated feature so for now, I will keep it like this
*/
__global__ void
dpc_kernel(FrameView<uint16_t> in, FrameView<uint16_t> out, DpcKernelParams params)
{
    const uint32_t x = blockIdx.x * blockDim.x + threadIdx.x;
    const uint32_t y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= in.width || y >= in.height)
        return;

    const uint32_t y_lo = (y >= 2) ? y - 2 : y + 2;
    const uint32_t y_hi = (y + 2 < in.height) ? y + 2 : y - 2;
    const uint32_t x_lo = (x >= 2) ? x - 2 : x + 2;
    const uint32_t x_hi = (x + 2 < in.width) ? x + 2 : x - 2;

    const int32_t curr = in.at(y, x);
    const int32_t top = in.at(y_lo, x);
    const int32_t bottom = in.at(y_hi, x);
    const int32_t left = in.at(y, x_lo);
    const int32_t right = in.at(y, x_hi);
    const int32_t top_left = in.at(y_lo, x_lo);
    const int32_t top_right = in.at(y_lo, x_hi);
    const int32_t bottom_left = in.at(y_hi, x_lo);
    const int32_t bottom_right = in.at(y_hi, x_hi);

    const int32_t threshold = params.diff_threshold;

    /*
       TODO:
       idk if its good in the IQ field to get the died pixels count or its useless info since
       there is nothing to do but changing the sensor. But maybe in the future i will implement
       this.
    */
    const bool is_dead =
        (abs(curr - top) > threshold) && (abs(curr - bottom) > threshold) &&
        (abs(curr - left) > threshold) && (abs(curr - right) > threshold) &&
        (abs(curr - top_left) > threshold) && (abs(curr - top_right) > threshold) &&
        (abs(curr - bottom_left) > threshold) && (abs(curr - bottom_right) > threshold);

    uint16_t result = curr;
    if (is_dead)
    {
        const int32_t vert_diff = abs(2 * curr - top - bottom);
        const int32_t hori_diff = abs(2 * curr - left - right);
        const int32_t left_diag_diff = abs(2 * curr - top_left - bottom_right);
        const int32_t right_diag_diff = abs(2 * curr - bottom_left - top_right);

        int32_t best = vert_diff;
        int32_t avg = (top + bottom) >> 1;

        if (hori_diff < best)
        {
            best = hori_diff;
            avg = (left + right) >> 1;
        }
        if (left_diag_diff < best)
        {
            best = left_diag_diff;
            avg = (top_left + bottom_right) >> 1;
        }
        if (right_diag_diff < best)
        {
            best = right_diag_diff;
            avg = (bottom_left + top_right) >> 1;
        }

        result = static_cast<uint16_t>(avg);
    }

    out.at(y, x) = result;
}

DpcBlock::DpcBlock(const IspConfig& cfg, cudaStream_t stream)
    : IspBlock(cfg, stream), params_(make_params(cfg))
{
}

void
DpcBlock::execute(PipelineData& data)
{
    if (!data.bayer)
    {
        throw std::runtime_error("DpcBlock: bayer frame is null");
    }

    auto& frame = *data.bayer;
    auto out =
        std::make_unique<PitchedFrame<uint16_t>>(frame.width(), frame.height(), frame.channels());

    const dim3 block(16, 16);
    const dim3 grid((frame.width() + block.x - 1) / block.x,
                    (frame.height() + block.y - 1) / block.y);

    dpc_kernel<<<grid, block, 0, stream_>>>(frame.view(), out->view(), params_);

    data.bayer = std::move(out);
}
