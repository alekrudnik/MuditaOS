#pragma once

#include <cstdint>
#include <memory>
#include <utility>

namespace audio
{
    class Stream
    {
      public:
        struct Span
        {
            std::uint8_t *data   = nullptr;
            std::size_t dataSize = 0;
        };

        static constexpr auto defaultBufferingSize = 4U;

        Stream(std::size_t blockSize, unsigned int bufferingSize = defaultBufferingSize)
            : _blockSize(blockSize), _buffer(std::make_unique<std::uint8_t[]>(blockSize * bufferingSize)),
              _emptyBuffer(std::make_unique<std::uint8_t[]>(blockSize))
        {
            std::fill(_emptyBuffer.get(), _emptyBuffer.get() + blockSize, 0);
        }

        bool push(void *data, std::size_t dataSize);
        bool push(const Span &span)
        {
            return push(span.data, span.dataSize);
        }

        bool reserve(Span &span);

        bool peek(Span &span);

        bool pop();
        bool pop(Span &span)
        {
            return peek(span) && pop();
        }

        std::size_t getBlockSize() const noexcept
        {
            return _blockSize;
        }

      private:
        std::size_t _blockSize = 0;
        std::unique_ptr<uint8_t[]> _buffer;
        std::unique_ptr<uint8_t[]> _emptyBuffer;
    };

} // namespace audio
