#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <cuda_runtime.h>

/*
   For convenience, both FrameView and PitchedFrame uses (row, column) accessing
   hence y and x, not x and y. Usually, x y make more sense but in this context, row col is the way
*/
template <typename T> struct FrameView
{
    T* data;
    // pitch will hold the raw stride size in bytes (width + alignment)
    size_t pitch;
    uint32_t width;
    uint32_t height;
    uint32_t channels;

    __host__ __device__ __forceinline__ T*
    row_ptr(uint32_t y) const
    {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(data) + y * pitch);
    }

    // device-side accessors
    __host__ __device__ __forceinline__ T&
    at(uint32_t y, uint32_t x) const
    {
        return row_ptr(y)[x];
    }

    __host__ __device__ __forceinline__ T&
    at(uint32_t y, uint32_t x, uint32_t c) const
    {
        return row_ptr(y)[x * channels + c];
    }
};

template <typename T> class PitchedFrame
{
  public:
    static constexpr size_t ALIGN = 256;

    PitchedFrame() = default;

    PitchedFrame(uint32_t width, uint32_t height, uint32_t channels = 1)
        : width_(width), height_(height), channels_(channels)
    {
        const size_t row_bytes = width * channels * sizeof(T);
        pitch_ = align_up(row_bytes, ALIGN);

        cudaError_t err = cudaMallocManaged(&data_, pitch_ * height);
        if (err != cudaSuccess)
            throw std::runtime_error(std::string("cudaMallocManaged failed: ") +
                                     cudaGetErrorString(err));
    }

    ~PitchedFrame()
    {
        if (data_)
            cudaFree(data_);
    }

    PitchedFrame(const PitchedFrame&) = delete;
    PitchedFrame& operator=(const PitchedFrame&) = delete;

    PitchedFrame(PitchedFrame&& o) noexcept
        : data_(o.data_),
          pitch_(o.pitch_),
          width_(o.width_),
          height_(o.height_),
          channels_(o.channels_)
    {
        o.data_ = nullptr;
    }

    PitchedFrame&
    operator=(PitchedFrame&& other) noexcept
    {
        if (this != &other)
        {
            if (data_)
                cudaFree(data_);
            data_ = other.data_;
            pitch_ = other.pitch_;
            width_ = other.width_;
            height_ = other.height_;
            channels_ = other.channels_;
            other.data_ = nullptr;
        }
        return *this;
    }

    FrameView<T>
    view() const
    {
        return { data_, pitch_, width_, height_, channels_ };
    }

    // it's (y.pitch + x) but i put it this way to assure 1-byte ptr arithmetic then cast it back to
    // whatever T is
    T&
    at(uint32_t y, uint32_t x)
    {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(data_) + y * pitch_)[x];
    }

    T&
    at(uint32_t y, uint32_t x, uint32_t c)
    {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(data_) + y * pitch_)[x * channels_ + c];
    }

    T*
    data() const
    {
        return data_;
    }
    size_t
    pitch() const
    {
        return pitch_;
    }
    uint32_t
    width() const
    {
        return width_;
    }
    uint32_t
    height() const
    {
        return height_;
    }
    uint32_t
    channels() const
    {
        return channels_;
    }

    size_t
    bytes() const
    {
        return pitch_ * height_;
    }

  private:
    static size_t
    align_up(size_t n, size_t align)
    {
        return (n + align - 1) / align * align;
    }

    T* data_ = nullptr;
    size_t pitch_ = 0;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    uint32_t channels_ = 0;
};
